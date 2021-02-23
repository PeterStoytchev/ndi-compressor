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

	ndiAudioHandler = std::thread([this] {
		NdiAudio();
	});
	ndiAudioHandler.detach();

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
		OPTICK_EVENT("NdiVideoCapture");
		DEBUG_LOG("[NdiVideoHandler] Starting new batch\n");

		m_ndiMutex.lock();
		DEBUG_LOG("[NdiVideoHandler] Got lock\n");

		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			auto video_frame = *m_ndiManager->CaptureVideoFrame();
			auto pkt = m_encoder->Encode(&video_frame);
			
			DEBUG_LOG("[NdiVideoHandler] Got frame and encoded it!\n");

			if (pkt != nullptr && pkt->size != 0)
			{
				m_recvBuffer->ndiVideoFrames[i] = video_frame;
				m_recvBuffer->encodedVideoPtrs[i] = pkt;
				m_recvBuffer->encodedVideoSizes[i] = pkt->size;

				DEBUG_LOG("[NdiVideoHandler] Frame was valid and was added to the FrameBuffer\n");
			}
			else
			{
				av_packet_free(&pkt);
				i--;

				DEBUG_LOG("[NdiVideoHandler] Frame was invalid and was freed, i--\n");
			}
		}

		DEBUG_LOG("[NdiVideoHandler] Batch capture complete, waiting on audio!\n");

		DEBUG_LOG("[NdiVideoHandler] atomic sync value: %i\n", m_atomicNdiSync.load());
		while (m_atomicNdiSync < 1); //wait on audio thread
		m_atomicNdiSync++;

		m_ndiMutex.unlock();
		
		DEBUG_LOG("[NdiVideoHandler] Audio is done, unlocking and waiting for condition_variable!\n");

		std::unique_lock<std::mutex> lk(m_cvMutex);
		m_cv.wait(lk);
	}
}

void FrameWrangler::NdiAudio()
{
	OPTICK_THREAD("NdiAudioThread");

	while (!m_exit)
	{
		OPTICK_EVENT("NdiAudioCapture");
		DEBUG_LOG("[NdiAudioHandler] Starting new batch\n");

		for (int i = 0; i < FRAME_BATCH_SIZE_AUDIO; i++)
		{
			m_recvBuffer->ndiAudioFrames[i] = *m_ndiManager->CaptureAudioFrame();

			DEBUG_LOG("[NdiAudioHandler] Captured a frame and added it to the FrameBuffer\n");
		}

		DEBUG_LOG("[NdiAudioHandler] atomic sync value: %i\n", m_atomicNdiSync.load());
		m_atomicNdiSync++;

		DEBUG_LOG("[NdiAudioHandler] Batch is done, signaled and waiting for condition_variable!\n");

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
		DEBUG_LOG("[Main] Starting new iteration! Waiting for confirmaton from client!\n");

		m_frameSender->WaitForConfirmation();
		DEBUG_LOG("[Main] Got confirmation! Waiting on lock.\n");

		m_ndiMutex.lock();

		while (m_atomicNdiSync < 2); //wait on capture threads
		m_atomicNdiSync = 0;

		DEBUG_LOG("[Main] Got lock!\n");

		//swap the two buffers
		FrameBuffer* local = m_sendingBuffer;
		m_sendingBuffer = m_recvBuffer;
		m_recvBuffer = local;

		m_ndiMutex.unlock();
		m_cv.notify_one();
		m_cv.notify_one();

		DEBUG_LOG("[Main] Swapped buffers and notified both Ndi threads!\n");
		DEBUG_LOG("[Main] Sending FrameBuffer to client!\n");

		m_frameSender->SendFrameBuffer(m_sendingBuffer);

		DEBUG_LOG("[Main] Buffer sent! Resetting buffer state.\n");

		//reset the buffer state
		free(m_sendingBuffer->packedData);

		m_sendingBuffer->totalDataSize = 0;
		m_sendingBuffer->totalAudioSize = 0;
		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			m_ndiManager->FreeVideo(&(m_sendingBuffer->ndiVideoFrames[i]));

			if (m_sendingBuffer->encodedVideoSizes[i] != 0)
				av_packet_free(&(m_sendingBuffer->encodedVideoPtrs[i]));

			m_sendingBuffer->encodedVideoSizes[i] = 0;
		}

		for (int i = 0; i < FRAME_BATCH_SIZE_AUDIO; i++)
		{
			m_ndiManager->FreeAudio(&(m_sendingBuffer->ndiAudioFrames[i]));
		}

		DEBUG_LOG("[Main] Buffer state reset!\n");
	}
}