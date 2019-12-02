#pragma once
#include "common_relation.h"
#include <stdio.h>
class encodepcmtoaac
{
public:
	encodepcmtoaac();
	~encodepcmtoaac();
	int encode_audio(const char* url, FILE* output_file);
};

