#include "FrameWrangler.h"
#include "Profiler.h"

FrameWrangler::FrameWrangler(NdiManager* ndiManager, FrameSender* frameSender, EncoderSettings encSettings)
{
	m_encoder = new Encoder(encSettings);

	this->m_ndiManager = ndiManager;
	this->m_frameSender = frameSender;
	
	ndiHandler = std::thread([this] {
		Ndi();
	});
	ndiHandler.detach();

	Main();
}

FrameWrangler::~FrameWrangler()
{
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

		m_ndiMutex.lock();

		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			auto video_frame = *m_ndiManager->CaptureVideoFrame();
			auto pkt = m_encoder->Encode(&video_frame);
			
			auto audio_frame = *m_ndiManager->CaptureAudioFrame();

			if (pkt != nullptr && pkt->size != 0)
			{
				m_recvBuffer->ndiVideoFrames[i] = video_frame;
				m_recvBuffer->encodedVideoPtrs[i] = pkt;
				m_recvBuffer->encodedVideoSizes[i] = pkt->size;

				m_recvBuffer->ndiAudioFrames[i] = audio_frame;
			}
			else
			{
				av_packet_free(&pkt);
				i--;
			}
		}

		m_ndiMutex.unlock();

		std::unique_lock<std::mutex> lk(m_cvMutex);
		m_cv.wait(lk);
	}
}

void FrameWrangler::Main()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	while (!m_exit)
	{
		OPTICK_FRAME("MainLoop");

		m_frameSender->WaitForConfirmation();

		m_ndiMutex.lock();

		//swap the two buffers
		FrameBuffer* local = m_sendingBuffer;
		m_sendingBuffer = m_recvBuffer;
		m_recvBuffer = local;

		m_ndiMutex.unlock();
		m_cv.notify_one();

		m_frameSender->SendFrameBuffer(m_sendingBuffer);

		//reset the buffer state
		free(m_sendingBuffer->packedData);

		m_sendingBuffer->totalDataSize = 0;
		m_sendingBuffer->totalAudioSize = 0;
		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			m_ndiManager->FreeVideo(&(m_sendingBuffer->ndiVideoFrames[i]));
			m_ndiManager->FreeAudio(&(m_sendingBuffer->ndiAudioFrames[i]));

			if (m_sendingBuffer->encodedVideoSizes[i] != 0)
				av_packet_free(&(m_sendingBuffer->encodedVideoPtrs[i]));

			m_sendingBuffer->encodedVideoSizes[i] = 0;
		}
	}
}