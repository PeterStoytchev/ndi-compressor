#pragma once

#include <libavformat/avformat.h>

#define FRAME_BATCH_SIZE 30

struct VideoPkt
{
	unsigned int frameSize = 0;
	NDIlib_video_frame_v2_t videoFrame;
	AVPacket* encodedDataPacket;
};

struct AudioFrame
{
	size_t dataSize;
	NDIlib_audio_frame_v2_t audioFrame;
};