
#include <string>
#include <iostream>
#include "transmuxer.h"
using namespace std;


int main()
{ 
	//转封装
	transmuxer t;
	t.transport_file("D:\\audioAndvideo\\transmuxer\\live.mp4", 
		"D:\\audioAndvideo\\transmuxer\\live.avi");
	return 0;
}