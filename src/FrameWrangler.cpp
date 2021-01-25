#include "FrameWrangler.h"
#include "Profiler.h"

FrameWrangler::FrameWrangler(NdiManager* ndiManager, FrameSender* frameSender, EncoderSettings encSettings)
{
	m_encoder = new Encoder(encSettings);

	this->m_ndiManager = ndiManager;
	this->m_frameSender = frameSender;

	mainHandler = std::thread([this] {
		Main();
	});
	mainHandler.detach();
}

FrameWrangler::~FrameWrangler()
{
	mainHandler.join();
}

void FrameWrangler::Stop()
{
	m_exit = true;
}

void FrameWrangler::Main()
{
	while (!m_exit)
	{
		PROFILE_FRAME("MainLoop");

		for (int i = 0; i < 30; i++)
		{
			auto video_frame = *m_ndiManager->CaptureVideoFrame();
			auto pkt = m_encoder->Encode(&video_frame);

			if (pkt != nullptr && pkt->size != 0)
			{
				video_pkt.videoFrames[i] = video_frame;
				video_pkt.encodedDataPackets[i] = pkt;
				video_pkt.frameSizes[i] = pkt->size;

				//video_pkt.encodedDataSize += pkt->size;
			}
			else
			{
				m_ndiManager->FreeVideo(&video_frame);
				av_packet_free(&pkt);
				i--;
			}
		}

		m_frameSender->WaitForConfirmation();

		m_frameSender->SendVideoFrame(&video_pkt);

		for (int i = 0; i < 30; i++)
		{
			m_ndiManager->FreeVideo(&(video_pkt.videoFrames[i]));
			av_packet_free(&(video_pkt.encodedDataPackets[i]));
		}
	}
}