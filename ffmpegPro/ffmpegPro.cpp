
#include <string>
#include <iostream>
#include "rtmp_save_file.h"
#include "Decoder.h"
#include "pcm_relation.h"
#include "yuvrelation.h"
#include "demuxer_realation.h"
#include "decodeaac.h"
using namespace std;

/*
保存网络流到本地
*/
int do_RTMP_SAVE() {

	RTMP_SAVE rtmp_save;
	rtmp_save.init();
	int ret = rtmp_save.openInputEnv("rtmp://39.107.138.4:1935/myapp/test");
	if (ret >= 0) {
		ret = rtmp_save.openOutpuEnv("D:\\test.ts");
	}
	if (ret < 0) goto ERROR;
	while (true) {
		auto packet = rtmp_save.ReadPackectFromSource();
		if (packet) {
			ret = rtmp_save.WritePacket(packet);
			if (ret >= 0) {
				cout << "write packet" << endl;
			}
			else {
				cout << "write packet failed" << endl;
			}
		}
	}
ERROR:
	while (true) {
		this_thread::sleep_for(chrono::seconds(100));
	}
	return 0;

}

/*
编码pcm数据
*/
int doWithPCM() {
	Decoder decoder;
	decoder.init();
	decoder.openOutpuEnv();
	decoder.ReadFrameFromSource();
	return 0;

}



int main()
{  
	/*
	*保存网络流
	*do_RTMP_SAVE();
	*/


	//doWithPCM();
	//simplest_pcm16le_split("D:/audioAndvideo/NocturneNo2inEflat_44.1k_s16le.pcm");
	//simplest_pcm16le_halfvolumeleft("D:/audioAndvideo/NocturneNo2inEflat_44.1k_s16le.pcm");
	//simplest_pcm16le_doublespeed("D:/audioAndvideo/NocturneNo2inEflat_44.1k_s16le.pcm");
	/*char s[] = "D:\\audioAndvideo\\sintel_640_360.yuv";
	char* url = s;
	simplest_yuv420_split(url, 640, 360, 1);*/
	//demuxer("D:/audioAndvideo/test.mp4");
	//decodeaac();
	printf("%d", 'abcd');

	
	return 0;
}