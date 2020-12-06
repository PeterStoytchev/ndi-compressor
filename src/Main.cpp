#include <csignal>
#include <thread>
#include <atomic>

#include "Encoder.h"
#include "NdiManager.h"
#include "FrameSender.h"

#include "Instrumentor.h"

static std::atomic<bool> exit_loop(false);
static void sigint_handler(int)
{
	exit_loop = true;
}


//refactor this
void VideoHandler(NdiManager* ndiManager, FrameSender* frameSender, EncoderSettings encSettings)
{
	uint8_t* bsBuffer = (uint8_t*)malloc(2);

	Encoder encoder(encSettings);

	uint8_t* sendingBuffer = (uint8_t*)malloc(encSettings.xres * encSettings.yres * 2);
	 
	unsigned long long frameCounter = 0;
	while (!exit_loop)
	{
		InstrumentationTimer timer("VideoHandler");

		NDIlib_video_frame_v2_t* video_frame;
		{
			InstrumentationTimer timer("CaptureVideoFrame");
			video_frame = ndiManager->CaptureVideoFrame();
		}
		
		size_t dataSize;
		uint8_t* data;
		{
			InstrumentationTimer timer("EncodeFrame");
			auto tuple = encoder.Encode(video_frame);
			dataSize = std::get<0>(tuple);
			data = std::get<1>(tuple);
		}

		if (dataSize != 0)
		{
			VideoFrame frame;
			frame.dataSize = dataSize;

			frame.videoFrame = *video_frame;

			{
				InstrumentationTimer timer("WaitForConfirmation");
				frameSender->WaitForConfirmation();
			}

			{
				InstrumentationTimer timer("SendVideoFrame");
				frameSender->SendVideoFrame(frame, data);
			}
		}
		else
		{
			NDIlib_video_frame_v2_t bsFrame = NDIlib_video_frame_v2_t(1,1);
			bsFrame.timecode = video_frame->timecode;
			bsFrame.timestamp = video_frame->timestamp;

			VideoFrame frame;
			frame.dataSize = 1;
			
			frame.videoFrame = bsFrame;

			{
				InstrumentationTimer timer("WaitForConfirmation");
				frameSender->WaitForConfirmation();
			}

			{
				InstrumentationTimer timer("SendVideoFrame");
				frameSender->SendVideoFrame(frame, bsBuffer);
				ndiManager->FreeVideo(&bsFrame);
			}
		}

		ndiManager->FreeVideo(video_frame);
		frameCounter++;
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


int main(int argc, char** argv)
{
	EncoderSettings encSettings;
	if (argc < 2)
	{
		encSettings = EncoderSettings("config.yaml");
	}
	else
	{
		encSettings = EncoderSettings(argv[1]);
	}

	signal(SIGINT, sigint_handler);

	FrameSender* frameSender = new FrameSender(encSettings.ipDest.c_str(), encSettings.videoPort, encSettings.audioPort);
	NdiManager* ndiManager = new NdiManager(encSettings.ndiSrcName.c_str(), nullptr); //create on the heap in order to avoid problems when accessing this from more than one thread

	Instrumentor::Get().BeginSession("ndi-compressor");

	std::thread handler(VideoHandler, ndiManager, frameSender, encSettings);
	AudioHandler(ndiManager, frameSender);

	handler.join();

	delete ndiManager;
	delete frameSender;
	Instrumentor::Get().EndSession();
}