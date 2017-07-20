#pragma once

#include <string>
namespace Utils
{
	void LogError(const char* str_1, const char* str_2 = nullptr);
};

struct Logger
{
private:
	int indent = 0;
public:
	void IncreaseIndent() { indent++; }
	void DecreaseIndent() { indent--; }
	void PrintLine(const char* msg_1, const char* msg_2);
};
