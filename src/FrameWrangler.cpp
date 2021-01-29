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
	video_pkts.reserve(FRAME_BATCH_SIZE);

	while (!m_exit)
	{
		OPTICK_FRAME("MainLoop");

		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			auto video_frame = *m_ndiManager->CaptureVideoFrame();
			auto pkt = m_encoder->Encode(&video_frame);

			if (pkt != nullptr && pkt->size != 0)
			{
				VideoPkt video_pkt;

				video_pkt.videoFrame = video_frame;
				video_pkt.encodedDataPacket = pkt;
				video_pkt.frameSize = pkt->size;

				video_pkts.push_back(video_pkt);
			}
			else
			{
				m_ndiManager->FreeVideo(&video_frame);
				av_packet_free(&pkt);
			}
		}

		m_frameSender->WaitForConfirmation();
		m_frameSender->SendVideoFrame(video_pkts);

		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			m_ndiManager->FreeVideo(&(video_pkts[i].videoFrame));
			av_packet_free(&(video_pkts[i].encodedDataPacket));
		}

		video_pkts.clear();
	}
}