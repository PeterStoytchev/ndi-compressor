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
	encSettings.bitrate = 2500000 * 2;

	Encoder encoder(encSettings);

	uint8_t* sendingBuffer = (uint8_t*)malloc(encSettings.xres * encSettings.yres * 4);

	while (!exit_loop)
	{
		NDIlib_video_frame_v2_t* video_frame = ndiManager->CaptureVideoFrame();
		NDIlib_video_frame_v2_t* video_frame2 = ndiManager->CaptureVideoFrame();
		
		auto [dataSize, data] = encoder.Encode(video_frame);
		memmove(sendingBuffer, data, dataSize);

		auto [dataSize2, data2] = encoder.Encode(video_frame2);

		if (dataSize != 0 && dataSize2 != 0)
		{
			VideoFramePair pair;
			pair.dataSize1 = dataSize;
			pair.dataSize2 = dataSize2;

			pair.videoFrame1 = *video_frame;
			pair.videoFrame2 = *video_frame2;

			memmove(sendingBuffer + dataSize, data2, dataSize2);

			frameSender->SendVideoFrame(pair, sendingBuffer);
		}
		else
		{
			NDIlib_video_frame_v2_t bsFrame = NDIlib_video_frame_v2_t(1,1);
			bsFrame.timecode = video_frame->timecode;
			bsFrame.timestamp = video_frame->timestamp;

			VideoFramePair pair;
			pair.dataSize1 = 2;
			pair.dataSize2 = 2;

			pair.videoFrame1 = bsFrame;
			pair.videoFrame2 = bsFrame;

			frameSender->SendVideoFrame(pair, bsBuffer);

			ndiManager->FreeVideo(&bsFrame);
		}

		ndiManager->FreeVideo(video_frame);
		ndiManager->FreeVideo(video_frame2);

		frameSender->WaitForConfirmation();

	}

	free(sendingBuffer);
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

	//FrameSender* frameSender = new FrameSender("213.214.65.185", 1337, 1338);
	FrameSender* frameSender = new FrameSender("192.168.1.106", 1337, 1338);
	NdiManager* ndiManager = new NdiManager("DESKTOP-G0O595D (wronghousefool)", nullptr); //create on the heap in order to avoid problems when accessing this from more than one thread

	std::thread handler(VideoHandler, ndiManager, frameSender);
	AudioHandler(ndiManager, frameSender);

	handler.join();

	delete ndiManager;
	delete frameSender;
}