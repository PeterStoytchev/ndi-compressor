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
			m_ndiMutex.lock();
			sameThreadLocked = true;
		}

		auto video_frame = *m_ndiManager->CaptureVideoFrame();
		auto pkt = m_encoder->Encode(&video_frame);

		if (pkt != nullptr && pkt->size != 0)
		{
			VideoPkt video_pkt;

			video_pkt.videoFrame = video_frame;
			video_pkt.encodedDataPacket = pkt;
			video_pkt.frameSize = pkt->size;

			m_ndiQueue.push_back(video_pkt);

			if (m_ndiQueue.size() > FRAME_BATCH_SIZE)
			{
				//free the first frame in the vector
				m_ndiManager->FreeVideo(&(m_ndiQueue[0].videoFrame));
				av_packet_free(&(m_ndiQueue[0].encodedDataPacket));

				//shift the vector one element to the front
				std::move(m_ndiQueue.begin() + 1, m_ndiQueue.end(), m_ndiQueue.begin());
				m_ndiQueue.pop_back();
			}
		}
		else
		{
			m_ndiManager->FreeVideo(&video_frame);
			av_packet_free(&pkt);
		}

		if (m_ndiQueue.size() == FRAME_BATCH_SIZE)
		{
			m_ndiMutex.unlock();
			sameThreadLocked = false;
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}

	}
}

void FrameWrangler::Main()
{
	while (!m_exit)
	{
		OPTICK_FRAME("MainLoop");

		m_frameSender->WaitForConfirmation();
		
		m_ndiMutex.lock();

		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			video_pkts.push_back(m_ndiQueue[i]);
		}

		m_ndiQueue.clear();
		
		m_ndiMutex.unlock();

		m_frameSender->SendVideoFrame(video_pkts);

		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			m_ndiManager->FreeVideo(&(video_pkts[i].videoFrame));
			av_packet_free(&(video_pkts[i].encodedDataPacket));
		}

		video_pkts.clear();
	}
}