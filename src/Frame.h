#pragma once

#include <libavformat/avformat.h>

#define FRAME_BATCH_SIZE 160

struct VideoPkt
{
	unsigned int frameSizes[FRAME_BATCH_SIZE];
	NDIlib_video_frame_v2_t videoFrames[FRAME_BATCH_SIZE];
	AVPacket* encodedDataPackets[FRAME_BATCH_SIZE];
};

struct AudioFrame
{
	size_t dataSize;
	NDIlib_audio_frame_v2_t audioFrame;
};