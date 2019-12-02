
#include <string>
#include <iostream>
#include "encodeyuvtojpg.h"
#include "encodepcmtoaac.h"
#include "encodeyuvtoh264.h"
using namespace std;



int main()
{   
	/*
	//yuv 编码为jg
	encodeyuvtojpg e;
	e.encode();
	return 0;
	*/
	/*
	pcm->aac
	*/
	/*
	encodepcmtoaac e;
	FILE* file = fopen("D:\\audioAndvideo\\encode\\test.mp3","wb+");
	e.encode_audio("D:\\audioAndvideo\\encode\\test.pcm",file);
	*/
	
	encodeyuvtoh264 e;
	e.encodeframetoh264("d:\\audioandvideo\\encode\\sintel_640_360.yuv",
		"d:\\audioandvideo\\encode\\sintel_640_360.h264");
		
	printf("%d", NULL);
}