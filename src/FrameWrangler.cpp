#include "FrameWrangler.h"
#include "optik/optick.h"

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
		OPTICK_FRAME("MainLoop");

		if (m_LastFrameGood)
			m_frameSender->WaitForConfirmation();

		VideoFrame video_frame;
		video_frame.videoFrame = *m_ndiManager->CaptureVideoFrame();

		video_frame.encodedDataPacket = m_encoder->Encode(&video_frame.videoFrame);

		if (video_frame.encodedDataPacket != nullptr && video_frame.encodedDataPacket->size != 0)
		{
			m_LastFrameGood = true;

			video_frame.encodedDataSize = video_frame.encodedDataPacket->size;

			m_frameSender->SendVideoFrame(&video_frame);

			m_ndiManager->FreeVideo(&(video_frame.videoFrame));
			av_packet_free(&(video_frame.encodedDataPacket));
		}
		else
		{
			m_ndiManager->FreeVideo(&(video_frame.videoFrame));
			m_LastFrameGood = false;
		}
	}
}