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
		if (m_shouldRun)
		{
			if (m_frameQueue.size() < 240)
			{
				OPTICK_EVENT();

				m_ndiMutex.try_lock();

				auto video_frame = *m_ndiManager->CaptureVideoFrame();
				auto pkt = m_encoder->Encode(&video_frame);

				if (pkt != nullptr && pkt->size != 0)
				{
					VideoPkt video_pkt;

					video_pkt.videoFrame = video_frame;
					video_pkt.encodedDataPacket = pkt;
					video_pkt.frameSize = pkt->size;

					m_frameQueue.push_back(video_pkt);
				}
				else
				{
					m_ndiManager->FreeVideo(&video_frame);
					av_packet_free(&pkt);
				}
			}
		}
		else
		{
			m_ndiMutex.unlock();
		
			std::unique_lock<std::mutex> lk(m_cvMutex);
			m_cv.wait(lk);
			
			lk.unlock();
		}
		
	}
}

void FrameWrangler::Main()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	while (!m_exit)
	{
		OPTICK_FRAME("MainLoop");

		m_frameSender->WaitForConfirmation();
		m_shouldRun = false;

		m_ndiMutex.lock();
		m_frameSender->SendVideoFrame(m_frameQueue);

		for (int i = 0; i < m_frameQueue.size(); i++)
		{
			m_ndiManager->FreeVideo(&(m_frameQueue[i].videoFrame));
			av_packet_free(&(m_frameQueue[i].encodedDataPacket));
		}

		m_frameQueue.clear();
		m_ndiMutex.unlock();
		
		m_shouldRun = true;
		m_cv.notify_one();
	}
}