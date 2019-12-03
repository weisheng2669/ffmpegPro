
#include <string>
#include <iostream>
#include "video_pic_goted.h"
#include "encodeyuvtojpg.h"
using namespace std;


int main()
{   
	/*
	video_pic_goted v;
	v.got_pic_from_video("http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8","D:\\audioAndvideo\\video_op\\test.jpg");
	*/

	encodeyuvtojpg e;
	e.encode("D:\\audioAndvideo\\encode\\sintel_640_360.yuv",
		"D:\\audioAndvideo\\encode\\sintel_640_360.jpg");
	return 0;
}