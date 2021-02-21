#pragma once

#include <libavformat/avformat.h>

#define FRAME_BATCH_SIZE 30

struct FrameBuffer
{
	size_t totalDataSize = 0;
	size_t encodedFrameSizes[FRAME_BATCH_SIZE];
	AVPacket* encodedFramePtrs[FRAME_BATCH_SIZE];
	NDIlib_video_frame_v2_t videoFrames[FRAME_BATCH_SIZE];
};

struct AudioFrame
{
	size_t dataSize;
	NDIlib_audio_frame_v2_t audioFrame;
};