#pragma once

#include "common_relation.h"


using namespace std;
class RTMP_SAVE
{
public:
	RTMP_SAVE();
	~RTMP_SAVE();
	void init();
	//获取输入的上下文环境,流信息等
	int openInputEnv(string inputUrl);
	//创建输出的上下文
	int openOutpuEnv(string outUrl);
	//读出Packet
	shared_ptr<AVPacket> ReadPackectFromSource();
	//写入文件
	int WritePacket(shared_ptr<AVPacket> packet);

private:

};










