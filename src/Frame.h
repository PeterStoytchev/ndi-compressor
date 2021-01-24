#pragma once

#include <libavformat/avformat.h>

struct VideoPkt
{
	size_t encodedDataSize = 0;
	
	int frameSizes[30];
	NDIlib_video_frame_v2_t videoFrames[30];
	AVPacket* encodedDataPackets[30];
};

struct AudioFrame
{
	size_t dataSize;
	NDIlib_audio_frame_v2_t audioFrame;
};