#include "utils.h"

#include <cstdio>

void Utils::LogError(const char* str_1, const char* str_2)
{
	fprintf_s(stderr, "Error: %s %s\n", str_1, str_2);
}
