#pragma once

#include <tuple>
#include <unordered_map>
#include "Processing.NDI.Lib.h"

#include "yaml-cpp/yaml.h"

#ifdef _DEBUG
#pragma comment(lib, "libyaml-debug")
#endif // _DEBUG

#ifndef _DEBUG
#pragma comment(lib, "libyaml-release")
#endif

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
	EncoderSettings() {}

	EncoderSettings(const EncoderSettings& settings) //this is here just to ensure that the priv data is copied right
	{
		encoderName = settings.encoderName;

		int64_t bitrateMbps = settings.bitrateMbps;

		AVPixelFormat pix_fmt = settings.pix_fmt;
		
		int gop_size = settings.gop_size;
		int max_b_frames = settings.max_b_frames;

		int xres = settings.xres;
		int yres = settings.yres;

		int fps = settings.fps;

		priv_data = settings.priv_data;
	}

	EncoderSettings(const char* path)
	{
		YAML::Node config = YAML::LoadFile(path);

		encoderName = config["encoderName"].as<std::string>();

		int64_t bitrateMbps = config["bitrateMbps"].as<int64_t>();

		int gop_size = config["gop_size"].as<int>();
		int max_b_frames = config["max_b_frames"].as<int>();

		int xres = config["xres"].as<int>();
		int yres = config["yres"].as<int>();

		int fps = config["fps"].as<int>();

		for (const auto& kv : config["priv_data"])
		{
			priv_data.emplace(kv.first.as<std::string>(), kv.second.as<std::string>());
		}
	}

	std::string encoderName = "h264_nvenc";
	int64_t bitrateMbps;

	AVPixelFormat pix_fmt = AV_PIX_FMT_NV12;
	int gop_size = 60;
	int max_b_frames = 0;

	int xres = 1920;
	int yres = 1080;

	int fps = 60;

	std::unordered_map<std::string, std::string> priv_data;
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