#include <csignal>
#include <thread>
#include <atomic>

#include "Encoder.h"
#include "NdiManager.h"
#include "FrameSender.h"


static std::atomic<bool> exit_loop(false);
static void sigint_handler(int)
{
	exit_loop = true;
}


void VideoHandler(NdiManager* ndiManager, FrameSender* frameSender)
{
	uint8_t* bsBuffer = (uint8_t*)malloc(2);
	EncoderSettings encSettings;
	encSettings.bitrate = 2500;

	Encoder encoder(encSettings);

	while (!exit_loop)
	{
		NDIlib_video_frame_v2_t* video_frame = ndiManager->CaptureVideoFrame();
		auto [dataSize, data] = encoder.Encode(video_frame);

		if (dataSize != 0)
		{
			frameSender->SendVideoFrame(video_frame, data, dataSize);
		}
		else
		{
			NDIlib_video_frame_v2_t bsFrame = NDIlib_video_frame_v2_t(1,1);
			bsFrame.timecode = video_frame->timecode;
			bsFrame.timestamp = video_frame->timestamp;

			frameSender->SendVideoFrame(&bsFrame, bsBuffer, 2);

			ndiManager->FreeVideo(&bsFrame);
		}

		ndiManager->FreeVideo(video_frame);

	}

}

void AudioHandler(NdiManager* ndiManager, FrameSender* frameSender)
{
	while (!exit_loop)
	{
		NDIlib_audio_frame_v2_t* audio_frame = ndiManager->CaptureAudioFrame();

		frameSender->SendAudioFrame(audio_frame);

		ndiManager->FreeAudio(audio_frame);
	}
}


int main()
{
	signal(SIGINT, sigint_handler);

	FrameSender* frameSender = new FrameSender("192.168.1.102", 1337, 1338);
	NdiManager* ndiManager = new NdiManager("NDISource", nullptr); //create on the heap in order to avoid problems when accessing this from more than one thread
	

	std::thread handler(VideoHandler, ndiManager, frameSender);
	AudioHandler(ndiManager, frameSender);

	handler.join();

	delete ndiManager;
	delete frameSender;
}