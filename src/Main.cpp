#include <csignal>
#include <thread>
#include <atomic>

#include "NdiManager.h"
#include "Encoder.h"

static std::atomic<bool> exit_loop(false);
static void sigint_handler(int)
{
	exit_loop = true;
}


void VideoHandler(NdiManager* ndiManager)
{
	EncoderSettings encSettings;
	encSettings.bitrate = 2500;

	Encoder encoder(encSettings);

	while (!exit_loop)
	{
		NDIlib_video_frame_v2_t* video_frame = ndiManager->CaptureVideoFrame();
		encoder.Encode(video_frame);

		printf("Send to server!\n");

		ndiManager->FreeVideo(video_frame);
	}

}

void AudioHandler(NdiManager* ndiManager)
{
	while (!exit_loop)
	{
		NDIlib_audio_frame_v2_t* audio_frame = ndiManager->CaptureAudioFrame();
	}
}


int main()
{
	signal(SIGINT, sigint_handler);

	NdiManager* ndiManager = new NdiManager("NDISource", nullptr); //create on the heap in order to avoid problems when accessing this from more than one thread

	std::thread handler(VideoHandler, ndiManager);
	AudioHandler(ndiManager);

	handler.join();

	delete ndiManager;
}