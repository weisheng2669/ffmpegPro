
#include <string>
#include <iostream>
#include "video_pic_goted.h"
using namespace std;


int main()
{ 
	video_pic_goted v;
	v.got_pic_from_video("http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8","D:\\audioAndvideo\\video_op\\test.jpg");

	return 0;
}