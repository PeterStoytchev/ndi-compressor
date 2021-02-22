#pragma once

#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

#include "Processing.NDI.Lib.h"

#include "Encoder.h"
#include "Frame.h"
#include "NdiManager.h"
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
	
	FrameBuffer* m_recvBuffer = new FrameBuffer();
	FrameBuffer* m_sendingBuffer = new FrameBuffer();

	std::mutex m_ndiMutex;
	std::mutex m_cvMutex;
	std::condition_variable m_cv;

	std::thread ndiHandler;
	std::thread mainHandler;

	std::atomic<bool> m_exit = false;
};