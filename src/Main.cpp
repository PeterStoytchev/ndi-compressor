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
	encSettings.bitrateMbps = 25;

	Encoder encoder(encSettings);

	uint8_t* sendingBuffer = (uint8_t*)malloc(encSettings.xres * encSettings.yres * 2);
	 
	int counter = 0;
	int diviser = 7;
	std::chrono::time_point<std::chrono::steady_clock> startPoint;

	while (!exit_loop)
	{
		if (counter % diviser == 0)
			startPoint = std::chrono::high_resolution_clock::now();

		NDIlib_video_frame_v2_t* video_frame = ndiManager->CaptureVideoFrame();
		
		auto [dataSize, data] = encoder.Encode(video_frame);

		if (dataSize != 0)
		{
			VideoFrame frame;
			frame.dataSize = dataSize;

			frame.videoFrame = *video_frame;

			frameSender->WaitForConfirmation();

			frameSender->SendVideoFrame(frame, data);
		}
		else
		{
			NDIlib_video_frame_v2_t bsFrame = NDIlib_video_frame_v2_t(1,1);
			bsFrame.timecode = video_frame->timecode;
			bsFrame.timestamp = video_frame->timestamp;

			VideoFrame frame;
			frame.dataSize = 1;
			
			frame.videoFrame = bsFrame;

			frameSender->WaitForConfirmation();

			frameSender->SendVideoFrame(frame, bsBuffer);
			ndiManager->FreeVideo(&bsFrame);
		}

		ndiManager->FreeVideo(video_frame);

		if (counter % diviser == 0)
		{
			long long RTT = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startPoint).count();
			
			if (RTT > 15)
			{
				std::cout << "RTT OVERBUGET BY: " << RTT - 15 << "ms\n";
			}
			counter = 0;
		}
		else
		{
			counter++;
		}
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

	//FrameSender* frameSender = new FrameSender("10.6.0.3", 1337, 1338);
	FrameSender* frameSender = new FrameSender("192.168.1.106", 1337, 1338);
	NdiManager* ndiManager = new NdiManager("DESKTOP-G0O595D (wronghousefool)", nullptr); //create on the heap in order to avoid problems when accessing this from more than one thread

	std::thread handler(VideoHandler, ndiManager, frameSender);
	AudioHandler(ndiManager, frameSender);

	handler.join();

	delete ndiManager;
	delete frameSender;
}