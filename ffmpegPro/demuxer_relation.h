#pragma once
#include "common_relation.h"
class demuxer_relation {
public:
	demuxer_relation();
	~demuxer_relation();
	int demuxer(const char* input_fname);

};