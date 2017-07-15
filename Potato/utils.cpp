#include "utils.h"

#include <cstdio>

void LogError(const std::string& error_log)
{
	fprintf_s(stderr, "Error: %s\n", error_log.c_str());
}
