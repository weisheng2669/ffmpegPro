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
	//���������������
	int openOutpuEnv();
	//����Frame
	int ReadFrameFromSource();
	//����
	void decodeFrame();
	//д���ļ�
	int WritePacket(shared_ptr<AVPacket> packet);

private:
};

