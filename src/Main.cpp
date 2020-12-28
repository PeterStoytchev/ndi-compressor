#include <csignal>
#include <thread>
#include <atomic>

#include "Encoder.h"
#include "NdiManager.h"
#include "FrameSender.h"

#include "FrameWrangler.h"

FrameWrangler* wrangler;

static std::atomic<bool> exit_loop(false);
static void sigint_handler(int)
{
	wrangler->Stop();
	exit_loop = true;
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
	NdiManager* ndiManager = new NdiManager(encSettings.ndiSrcName.c_str(), nullptr);
	wrangler = new FrameWrangler(ndiManager, frameSender, encSettings);
	
	
	AudioHandler(ndiManager, frameSender);

	delete wrangler;
	delete ndiManager;
	delete frameSender;
}