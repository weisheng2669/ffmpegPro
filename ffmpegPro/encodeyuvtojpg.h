#pragma once
#include "common_relation.h"
class encodeyuvtojpg
{
public:
	encodeyuvtojpg();
	~encodeyuvtojpg();
	int encode(const char* src_url,const char* dst_url);
};

