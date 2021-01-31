#pragma once

#include <libavformat/avformat.h>

#define FRAME_BATCH_SIZE 240

struct VideoPkt
{
	unsigned int frameSize = 0;
	NDIlib_video_frame_v2_t videoFrame = NDIlib_video_frame_v2_t();
	AVPacket* encodedDataPacket;
};

struct AudioFrame
{
	size_t dataSize;
	NDIlib_audio_frame_v2_t audioFrame;
};