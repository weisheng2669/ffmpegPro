#pragma once
#include "common_relation.h"
class transmuxer
{
public: 
	transmuxer();
	~transmuxer();
	int transport_file(const char* src_file,const char* dst_file);

private:

};

