#pragma once

#include <tuple>
#include "Processing.NDI.Lib.h"

extern "C"
{
	#include <math.h>
	#include <libavutil/opt.h>
	#include <libswscale/swscale.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/common.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/samplefmt.h>

	#pragma comment(lib, "avcodec")
	#pragma comment(lib, "avutil")
	#pragma comment(lib, "swscale")
}

#define LOG_ERR(ret) av_strerror(ret, errorBuf, 500); printf("&s\n", errorBuf); memset(errorBuf, 0, 500);

struct EncoderSettings
{
	const char* encoderName = "hevc_nvenc";
	int64_t bitrate;

	AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;
	int gop_size = 30;
	int max_b_frames = 0;

	int xres = 1920;
	int yres = 1080;

	int fps = 60;
};

class Encoder
{
public:
	Encoder(EncoderSettings settings);
	
	std::tuple<size_t, uint8_t*> Encode(NDIlib_video_frame_v2_t* frame);
private:
	EncoderSettings m_settings;
	int ret, i;

	AVCodec* codec;
	AVCodecContext * codecContext = NULL;
	
	AVFrame* frame;
	AVPacket* pkt;
	
	struct SwsContext* swsContext = NULL;

	char* errorBuf;
};