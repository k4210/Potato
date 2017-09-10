#pragma once

namespace Utils
{
	void LogSuccess(const char* str_0, const char* str_1 = "", const char* str_2 = "");
	void Log(const char* str_0, const char* str_1 = "", const char* str_2 = "");
	void LogError(const char* str_0, const char* str_1 = "", const char* str_2 = "");
	bool SoftAssert(bool condition, const char* error_msg = "");
};

struct Logger
{
private:
	int indent = 0;
public:
	void IncreaseIndent() { indent++; }
	void DecreaseIndent() { indent--; }
	void PrintLine(const char* msg_0, const char* msg_1, const char* msg_2 = "");
};

template<typename E>
struct Flag32
{
private:
	unsigned int data = 0;
public:

	bool Get(E v) const
	{
		return 0 != (data & static_cast<unsigned int>(v));
	}

	void Add(E v)
	{
		data |= static_cast<unsigned int>(v);
	}

	template <class... T>
	void Add(E head, T... tail)
	{
		Add(head);
		Add(tail...);
	}

	Flag32 operator|(Flag32 other) const
	{
		Flag32 new_flag(*this);
		new_flag.data |= other.data;
		return new_flag;
	}

	void Remove(E v)
	{
		data &= ~static_cast<unsigned int>(v);
	}
	void Reset()
	{
		data = 0;
	}

public:
	Flag32() = default;

	template <class... T>
	Flag32(T... tail) 
		: Flag32()
	{
		Add(tail...);
	}
};
