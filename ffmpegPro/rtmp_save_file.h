#pragma once

#include "common_relation.h"


using namespace std;
class RTMP_SAVE
{
public:
	RTMP_SAVE();
	~RTMP_SAVE();
	void init();
	//��ȡ����������Ļ���,����Ϣ��
	int openInputEnv(string inputUrl);
	//���������������
	int openOutpuEnv(string outUrl);
	//����Packet
	shared_ptr<AVPacket> ReadPackectFromSource();
	//д���ļ�
	int WritePacket(shared_ptr<AVPacket> packet);

private:

};










