#include "stdafx.h"
#include "utils.h"

#include <windows.h>
#include <cstdio>

void Utils::Log(const char* str_0, const char* str_1, const char* str_2)
{
	printf_s("%s %s %s\n", str_0, str_1, str_2);
}

void Utils::LogError(const char* str_0, const char* str_1, const char* str_2)
{
	HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
	fprintf_s(stderr, "Error: %s %s %s\n", str_0, str_1, str_2);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void Utils::LogSuccess(const char* str_0, const char* str_1, const char* str_2)
{
	HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	fprintf_s(stderr, "%s %s %s\n", str_0, str_1, str_2);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

bool Utils::SoftAssert(bool condition, const char* error_msg)
{
	if (!condition)
	{
		Utils::LogError("Assertion failed:", error_msg ? error_msg : "no details");
	}
	return condition;
}

void Logger::PrintLine(const char* msg_0, const char* msg_1, const char* msg_2)
{
	for (int i = 0; i < indent; ++i)
	{
		putchar('.');
	}
	printf_s("%s %s %s \n", msg_0, msg_1, msg_2);
}