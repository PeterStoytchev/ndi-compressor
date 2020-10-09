#include "NdiManager.h"

#include <chrono>
#include <thread>


void VideoHandler(NdiManager* ndiManager)
{
	using namespace std::chrono;
	for (const auto start = high_resolution_clock::now(); high_resolution_clock::now() - start < seconds(30);)
	{
		NDIlib_video_frame_v2_t video_frame = ndiManager->CaptureVideoFrame();
		ndiManager->SendAndFreeVideo(&video_frame);
	}
}

void AudioHandler(NdiManager* ndiManager)
{
	using namespace std::chrono;
	for (const auto start = high_resolution_clock::now(); high_resolution_clock::now() - start < seconds(30);)
	{
		NDIlib_audio_frame_v2_t audio_frame = ndiManager->CaptureAudioFrame();
		ndiManager->SendAndFreeAudio(&audio_frame);
	}
}


int main()
{
	NdiManager* ndiManager = new NdiManager("NDISource", "CompressedOutput"); //create on the heap in order to avoid problems when accessing this from more than one thread

	std::thread handler(VideoHandler, ndiManager);
	std::thread handler2(AudioHandler, ndiManager);

	handler.join();
	handler2.join();

	delete ndiManager;
}