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
		auto frame = m_ndiManager->CaptureVideoFrame();

		std::lock_guard<std::mutex> guard(m_ndiMutex);
		m_ndiQueue.push(frame);
		printf("DEBUG: Ndi queue size +1. Current size: %u\n", m_ndiQueue.size());
	}
}

void FrameWrangler::HandleSending()
{
	while (!m_exit)
	{
		m_encodingMutex.lock();
		if (!m_encodingQueue.empty())
		{
			auto frame = m_encodingQueue.front();
			m_encodingQueue.pop();

			m_encodingMutex.unlock();

			if (frame.dataSize != 0)
			{
				m_frameSender->SendVideoFrame(frame, frame.data);
			}
			else
			{
				NDIlib_video_frame_v2_t bsFrame = NDIlib_video_frame_v2_t(1, 1);
				bsFrame.timecode = frame.videoFrame.timecode;
				bsFrame.timestamp = frame.videoFrame.timestamp;

				frame.dataSize = 1;
				frame.videoFrame = bsFrame;
				frame.data = bsBuffer;

				m_frameSender->SendVideoFrame(frame, frame.data);
			}

			m_ndiManager->FreeVideo(&frame.videoFrame);
			free(frame.data);

			printf("DEBUG: Encoding queue size: %u\n", m_encodingQueue.size());
			m_frameSender->WaitForConfirmation();
		}
		else
		{
			m_encodingMutex.unlock();
		}
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

			auto [dataSize, data] = m_encoder->Encode(video_frame);

			VideoFrame frame(dataSize, data, video_frame);

			std::lock_guard<std::mutex> guard(m_encodingMutex);
			m_encodingQueue.push(frame);

			printf("DEBUG: Ndi queue size -1. Encoding current size: %u\n", m_encodingQueue.size());
		}
		else
		{
			m_ndiMutex.unlock();
		}
	}
}
