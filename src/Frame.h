#pragma once

#include <libavformat/avformat.h>

#define FRAME_BATCH_SIZE 30

struct FrameBuffer
{
	size_t totalDataSize = 0;
	size_t encodedFrameSizes[FRAME_BATCH_SIZE];
	AVPacket* encodedFramePtrs[FRAME_BATCH_SIZE];
	NDIlib_video_frame_v2_t videoFrames[FRAME_BATCH_SIZE];

	std::tuple<uint8_t*, size_t> PackData()
	{
		//compute total buffer size
		size_t dataSize = 0;
		for (int i = 0; i < FRAME_BATCH_SIZE; i++) { dataSize += encodedFrameSizes[i]; }
		totalDataSize = dataSize;

		uint8_t* frameData = (uint8_t*)malloc(dataSize); //data buffer used to hold all frame data

		size_t localSize = 0;
		//copy data into the buffer
		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			memcpy(frameData + localSize, encodedFramePtrs[i]->data, encodedFrameSizes[i]);
			localSize += encodedFrameSizes[i];
		}

		return std::make_tuple(frameData, dataSize);
	}
};

struct AudioFrame
{
	size_t dataSize;
	NDIlib_audio_frame_v2_t audioFrame;
};