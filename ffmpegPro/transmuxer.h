#pragma once
#include "common_relation.h"
class transmuxer
{
public: 
	transmuxer();
	~transmuxer();
	int transport_file(char* src_file, char* dst_file);

private:

};

