#pragma once

#include <libavformat/avformat.h>

struct VideoFrame
{
	size_t encodedDataSize = 0;
	AVPacket* encodedDataPacket;
	NDIlib_video_frame_v2_t videoFrame;
	uint64_t id = rand();
};

struct AudioFrame
{
	size_t dataSize;
	NDIlib_audio_frame_v2_t audioFrame;
};