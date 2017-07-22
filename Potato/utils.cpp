#include "stdafx.h"
#include "utils.h"

void Utils::LogError(const char* str_1, const char* str_2)
{
	fprintf_s(stderr, "Error: %s %s\n", str_1, str_2);
}

void Utils::LogError(std::string str_0, const char* str_1, const char* str_2)
{
	fprintf_s(stderr, "Error %s: %s %s\n", str_0.c_str(), str_1, str_2);
}

void Logger::PrintLine(const char* msg_1, const char* msg_2)
{
	for (int i = 0; i < indent; ++i)
	{
		putchar('.');
	}
	printf_s("%s %s \n", msg_1, msg_2);
}