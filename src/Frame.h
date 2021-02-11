#pragma once

#include <libavformat/avformat.h>

#define FRAME_BATCH_SIZE 30

struct VideoPktDetails
{
	size_t frameCount = 0;
	size_t dataSize = 0;
};

struct VideoPkt
{
	unsigned int frameSize = 0;
	AVPacket* encodedDataPacket;
	NDIlib_video_frame_v2_t videoFrame;
};

struct AudioFrame
{
	size_t dataSize;
	NDIlib_audio_frame_v2_t audioFrame;
};