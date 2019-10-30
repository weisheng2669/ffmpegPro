#pragma once
#include "common_relation.h"
using namespace std;

class Decoder
{
public:
	Decoder();
	~Decoder();
	void init();
	int flush_encoder(AVFormatContext* fmt_ctx, unsigned int stream_index);
	//创建输出的上下文
	int openOutpuEnv();
	//读出Frame
	int ReadFrameFromSource();
	//编码
	void decodeFrame();
	//写入文件
	int WritePacket(shared_ptr<AVPacket> packet);

private:
};

