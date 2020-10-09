#include "NdiManager.h"

#include <chrono>

int main()
{
	NdiManager* ndiManager = new NdiManager("NDISource", "CompressedOutput"); //create on the heap in order to avoid problems when accessing this from more than one thread

	using namespace std::chrono;
	for (const auto start = high_resolution_clock::now(); high_resolution_clock::now() - start < seconds(30);)
	{
		NDIlib_video_frame_v2_t video_frame;
		NDIlib_audio_frame_v2_t audio_frame;

		NDIlib_frame_type_e frameType = NDIlib_recv_capture_v2(*ndiManager->getRecever(), &video_frame, &audio_frame, nullptr, 5000);

		switch (frameType)
		{
			case NDIlib_frame_type_video:
				//printf("Got video! Resending!\n");
				NDIlib_send_send_video_v2(*ndiManager->getSender(), &video_frame);
				NDIlib_recv_free_video_v2(*ndiManager->getRecever(), &video_frame);
				break;

			case NDIlib_frame_type_audio:
				//printf("Got audio! Resending!\n");
				NDIlib_send_send_audio_v2(*ndiManager->getSender(), &audio_frame);
				NDIlib_recv_free_audio_v2(*ndiManager->getRecever(), &audio_frame);
				break;
		}
	}
}