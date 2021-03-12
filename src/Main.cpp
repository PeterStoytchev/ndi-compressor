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

	FrameSender* frameSender = new FrameSender(encSettings.ipDest.c_str(), encSettings.videoPort);
	NdiManager* ndiManager = new NdiManager(encSettings.ndiSrcName.c_str(), nullptr);
	wrangler = new FrameWrangler(ndiManager, frameSender, encSettings);
	
	delete wrangler;
	delete ndiManager;
	delete frameSender;
}