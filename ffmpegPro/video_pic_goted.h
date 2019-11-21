#pragma once
#include "common_relation.h"
#include <string>
#include <thread>
#include <memory>
#include <iostream>

using namespace std;

class video_pic_goted
{
public:
	video_pic_goted();
	~video_pic_goted();
	int got_pic_from_video(const char* src_url, const char* dst_url);
};

