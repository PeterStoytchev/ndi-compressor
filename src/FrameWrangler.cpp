#include "FrameWrangler.h"

FrameWrangler::FrameWrangler(NdiManager* ndiManager, FrameSender* frameSender, EncoderSettings encSettings)
{
	m_encoder = new Encoder(encSettings);

	this->m_ndiManager = ndiManager;
	this->m_frameSender = frameSender;

	ndiHandler = std::thread([this] {
		HandleNdi();
	});
	ndiHandler.detach();

	encodingHandler = std::thread([this] {
		HandleEncoding();
	});
	encodingHandler.detach();

	sendingHandler = std::thread([this] {
		HandleSending();
	});
	sendingHandler.detach();
}

FrameWrangler::~FrameWrangler()
{
	ndiHandler.join();
	sendingHandler.join();
	encodingHandler.join();
}

void FrameWrangler::Stop()
{
	m_exit = true;
}

void FrameWrangler::HandleNdi()
{
	while (!m_exit)
	{
		VideoFrame vframe;
		vframe.videoFrame = *m_ndiManager->CaptureVideoFrame();

		std::lock_guard<std::mutex> guard(m_ndiMutex);
		m_ndiQueue.push(vframe);
	}
}

void FrameWrangler::HandleEncoding()
{
	while (!m_exit)
	{
		m_ndiMutex.lock();
		if (!m_ndiQueue.empty())
		{
			auto video_frame = m_ndiQueue.front();
			
			m_ndiQueue.pop();
			m_ndiMutex.unlock();

			video_frame.encodedDataPacket = m_encoder->Encode(&video_frame.videoFrame);

			if (video_frame.encodedDataPacket != nullptr && video_frame.encodedDataPacket->size != 0)
			{
				std::lock_guard<std::mutex> guard(m_encodingMutex);
				m_encodingQueue.push(video_frame);
			}
			else
			{
				m_ndiManager->FreeVideo(&(video_frame.videoFrame));
			}
		}
		else
		{
			m_ndiMutex.unlock();
		}

	}
}

void FrameWrangler::HandleSending()
{
	while (!m_exit)
	{
		m_encodingMutex.lock();
		if (!m_encodingQueue.empty())
		{
			auto video_frame = m_encodingQueue.front();
			m_encodingQueue.pop();

			m_encodingMutex.unlock();

			video_frame.encodedDataSize = video_frame.encodedDataPacket->size;

			m_frameSender->SendVideoFrame(&video_frame);

			m_ndiManager->FreeVideo(&(video_frame.videoFrame));
			av_packet_free(&(video_frame.encodedDataPacket));
		}
		else
		{
			m_encodingMutex.unlock();
		}
	}
}