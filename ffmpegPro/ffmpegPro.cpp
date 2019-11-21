
#include <string>
#include <iostream>
#include "transmuxer.h"
#include "demuxer_relation.h"
#include "muxer.h"
using namespace std;


int main()
{ 
	//转封装
	/*
	transmuxer t;
	t.transport_file("D:\\audioAndvideo\\transmuxer\\live.mp4", 
		"D:\\audioAndvideo\\transmuxer\\live.avi");
    */
	//解封装
	/*
	demuxer_relation d;
	d.demuxer("D:\\audioAndvideo\\demuxer\\test.mp4");
	*/
	//封装
	muxer m;
	m.muxer_file("D:\\audioAndvideo\\muxer\\test_de.h264","D:\\audioAndvideo\\muxer\\test_de.mp3",
		"D:\\audioAndvideo\\muxer\\test_de.mp4");

	return 0;
}