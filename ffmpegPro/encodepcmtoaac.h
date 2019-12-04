#pragma once
#include "common_relation.h"
#include <stdio.h>
class encodepcmtoaac
{
public:
	encodepcmtoaac();
	~encodepcmtoaac();
	int encode_audio(const char* src_url,const char* dst_url);
};

