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
		ipDest = settings.ipDest;

		encoderName = settings.encoderName;

		ndiSrcName = settings.ndiSrcName;

		bitrateMbps = settings.bitrateMbps;

		videoPort = settings.videoPort;
		audioPort = settings.audioPort;

		pix_fmt = settings.pix_fmt;
		
		gop_size = settings.gop_size;
		max_b_frames = settings.max_b_frames;

		xres = settings.xres;
		yres = settings.yres;

		fps = settings.fps;

		thread_count = settings.thread_count;

		priv_data = settings.priv_data;
	}

	EncoderSettings(const char* path)
	{
		YAML::Node config = YAML::LoadFile(path);

		ipDest = config["ipDest"].as<std::string>();

		encoderName = config["encoderName"].as<std::string>();

		ndiSrcName = config["ndiSrcName"].as<std::string>();

		bitrateMbps = config["bitrateMbps"].as<int64_t>();

		gop_size = config["gop_size"].as<int>();
		max_b_frames = config["max_b_frames"].as<int>();

		videoPort = config["videoPort"].as<int>();
		audioPort = config["audioPort"].as<int>();

		xres = config["xres"].as<int>();
		yres = config["yres"].as<int>();

		fps = config["fps"].as<int>();

		thread_count = config["thread_count"].as<int>();

		for (const auto& kv : config["priv_data"])
		{
			priv_data.emplace(kv.first.as<std::string>(), kv.second.as<std::string>());
		}
	}

	std::string ipDest;
	std::string ndiSrcName;
	std::string encoderName;
	std::unordered_map<std::string, std::string> priv_data;

	int64_t bitrateMbps;
	AVPixelFormat pix_fmt = AV_PIX_FMT_NV12;
	
	int videoPort, audioPort, thread_count, xres, yres, fps, gop_size, max_b_frames;
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