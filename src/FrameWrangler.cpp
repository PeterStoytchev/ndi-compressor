#include "FrameWrangler.h"
#include "Profiler.h"

FrameWrangler::FrameWrangler(NdiManager* ndiManager, FrameSender* frameSender, EncoderSettings encSettings)
{
	m_encoder = new Encoder(encSettings);

	this->m_ndiManager = ndiManager;
	this->m_frameSender = frameSender;
	
	video_pkts.reserve(FRAME_BATCH_SIZE);
	m_ndiQueue.reserve(FRAME_BATCH_SIZE);

	mainHandler = std::thread([this] {
		Main();
	});
	mainHandler.detach();

	ndiHandler = std::thread([this] {
		Ndi();
	});
	ndiHandler.detach();
}

FrameWrangler::~FrameWrangler()
{
	mainHandler.join();
}

void FrameWrangler::Stop()
{
	m_exit = true;
}

void FrameWrangler::Ndi()
{
	OPTICK_THREAD("NdiThread");

	while (!m_exit)
	{
		OPTICK_EVENT();

		if (!sameThreadLocked)
		{
			OPTICK_EVENT("Locking");
			m_ndiMutex.lock();
			sameThreadLocked = true;
		}

		auto video_frame = m_ndiManager->CaptureVideoFrame();
		m_ndiQueue.push_back(video_frame);

		if (m_ndiQueue.size() > FRAME_BATCH_SIZE)
		{
			OPTICK_EVENT("ShiftQueue");
			//free the first frame in the vector
			m_ndiManager->FreeVideo(m_ndiQueue[0]);

			//shift the vector one element to the front
			std::move(m_ndiQueue.begin() + 1, m_ndiQueue.end(), m_ndiQueue.begin());
			m_ndiQueue.pop_back();
		}
		
		if (m_ndiQueue.size() == FRAME_BATCH_SIZE)
		{
			OPTICK_EVENT("GiveChanceForLock");
			m_ndiMutex.unlock();
			std::this_thread::sleep_for(std::chrono::microseconds(10));
			sameThreadLocked = false;
		}
	}
}

void FrameWrangler::Main()
{
	while (!m_exit)
	{
		OPTICK_FRAME("MainLoop");

		m_frameSender->WaitForConfirmation();
		
		{
			OPTICK_EVENT("AquireQueue");
			m_ndiMutex.lock();
		}

		{
			OPTICK_EVENT("QueueCopyAndUnlock");

			for (int i = 0; i < FRAME_BATCH_SIZE; i++) { m_workingQueue.push_back(m_ndiQueue[i]); }
			m_ndiQueue.clear();

			m_ndiMutex.unlock();
		}


		for (NDIlib_video_frame_v2_t* frame : m_workingQueue)
		{
			auto pkt = m_encoder->Encode(frame);

			VideoPkt video_pkt;
			if (pkt != nullptr && pkt->size != 0)
			{
				video_pkt.videoFrame = *frame;
				video_pkt.encodedDataPacket = pkt;
				video_pkt.frameSize = pkt->size;
			}
			else
			{
				m_ndiManager->FreeVideo(frame);
				av_packet_free(&pkt);
			}

			video_pkts.push_back(video_pkt);
		}
		m_workingQueue.clear();

		m_frameSender->SendVideoFrame(video_pkts);

		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			if (video_pkts[i].frameSize != 0)
			{
				m_ndiManager->FreeVideo(&(video_pkts[i].videoFrame));
				av_packet_free(&(video_pkts[i].encodedDataPacket));
			}
		}

		video_pkts.clear();
	}
}