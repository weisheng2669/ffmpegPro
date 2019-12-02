#pragma once
#include "common_relation.h"
class encodeyuvtoh264
{
public:
	encodeyuvtoh264();
	~encodeyuvtoh264();
	int encodeframetoh264(const char* src_url, const char* dst_url);
};

