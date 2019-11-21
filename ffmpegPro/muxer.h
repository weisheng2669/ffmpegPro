#pragma once
#include "common_relation.h"
class muxer
{
public:
	muxer();
	~muxer();
	int muxer_file(const char* src_url_v, const char* src_url_a, const char* dst_url);
private:

};


