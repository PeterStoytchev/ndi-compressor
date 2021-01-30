#pragma once

#include <queue>
#include <mutex>
#include <atomic>
#include <thread>

#include "Processing.NDI.Lib.h"

#include "Encoder.h"
#include "Frame.h"
#include "NdiManager.h"
#include "FrameQueue.h"
#include "FrameSender.h"

class FrameWrangler
{
public:
	FrameWrangler(NdiManager* ndiManager, FrameSender* frameSender, EncoderSettings encSettings);
	~FrameWrangler();

	void Stop();

	void Ndi();
	void Main();

private:
	Encoder* m_encoder;
	NdiManager* m_ndiManager;
	FrameSender* m_frameSender;
	
	std::vector<VideoPkt> m_ndiQueue;

	std::mutex m_ndiMutex;

	std::thread ndiHandler;
	std::thread mainHandler;

	std::vector<VideoPkt> video_pkts;

	std::atomic<bool> m_exit = false;

	bool sameThreadLocked = false;
};