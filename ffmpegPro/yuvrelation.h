#pragma once
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "common_relation.h"

using namespace std;

int simplest_yuv420_split(char* url, int w, int h, int num);
int decodeyuv420(char* url);

