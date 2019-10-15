
#include <string>
#include <iostream>
#include "rtmp_save_file.h"
using namespace std;

RTMP_SAVE rtmp_save;
int do_RTMP_SAVE() {
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

int main()
{    
	//保存网络流
	do_RTMP_SAVE();
	return 0;
}