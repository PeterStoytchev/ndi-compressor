#include "FrameWrangler.h"
#include "Profiler.h"

FrameWrangler::FrameWrangler(NdiManager* ndiManager, FrameSender* frameSender, EncoderSettings encSettings)
{
	m_encoder = new Encoder(encSettings);

	this->m_ndiManager = ndiManager;
	this->m_frameSender = frameSender;
	
	m_frameQueue.reserve(FRAME_BATCH_SIZE);

	ndiHandler = std::thread([this] {
		Ndi();
	});
	ndiHandler.detach();

	std::this_thread::sleep_for(std::chrono::milliseconds(1)); //kinda hacky, couldnt think of anything else, fight me

	mainHandler = std::thread([this] {
		Main();
	});
	mainHandler.detach();
}

FrameWrangler::~FrameWrangler()
{
	mainHandler.join();
	ndiHandler.join();
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

		if (!m_isLocked) 
		{ 
			m_ndiMutex.lock();
			m_isLocked = true;
		}

		auto video_frame = m_ndiManager->CaptureVideoFrame();
		m_frameQueue.push_back(video_frame);

		if (m_frameQueue.size() > FRAME_BATCH_SIZE)
		{
			m_ndiManager->FreeVideo(m_frameQueue[0]);

			std::move(m_frameQueue.begin() + 1, m_frameQueue.end(), m_frameQueue.begin());
			m_frameQueue.pop_back();

			m_isLocked = false;

			m_ndiMutex.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
		}
	}
}

void FrameWrangler::Main()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	while (!m_exit)
	{
		OPTICK_FRAME("MainLoop");

		m_frameSender->WaitForConfirmation();

		//this sucks
		while (true)
		{
			m_ndiMutex.lock();
			if (m_frameQueue.size() < FRAME_BATCH_SIZE) 
			{
				m_ndiMutex.unlock();
			}
			else
			{
				break;
			}
		}

		//copy the frames over
		for (int i = 0; i < m_frameQueue.size(); i++)
		{
			m_encodingQueue.push_back(m_frameQueue[i]);
		}

		m_frameQueue.clear();
		m_ndiMutex.unlock();

		//encode the frames
		for (int i = 0; i < m_encodingQueue.size(); i++)
		{
			auto pkt = m_encoder->Encode(m_encodingQueue[i]);

			if (pkt != nullptr && pkt->size != 0)
			{
				VideoPkt video_pkt;
				video_pkt.encodedDataPacket = pkt;
				video_pkt.videoFrame = *m_encodingQueue[i];
				video_pkt.frameSize = pkt->size;

				m_sendingQueue.push_back(video_pkt);
			}
			else
			{
				av_packet_free(&pkt);
			}
		}

		m_frameSender->SendVideoFrame(m_sendingQueue);

		for (int i = 0; i < m_encodingQueue.size(); i++)
		{
			m_ndiManager->FreeVideo(m_encodingQueue[i]);
		}

		for (int i = 0; i < m_sendingQueue.size(); i++)
		{
			if (m_sendingQueue[i].frameSize != 0)
				av_packet_free(&(m_sendingQueue[i].encodedDataPacket));
		}

		m_encodingQueue.clear();
		m_sendingQueue.clear();
	}
}