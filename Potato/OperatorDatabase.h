#pragma once

#include <map>
#include "Lexer.h"
#include "PotatoCommon.h"

struct OperatorId
{
	static const int kNameMaxSize = 4;
private:
	char name[kNameMaxSize];
public:
	operator const char*() const 
	{ 
		return name; 
	}
	OperatorId(const char* in_name);
};

class BinaryOperatorDatabase
{
public:
	static const BinaryOperatorDatabase& Get();

	struct OperatorData
	{
		OperatorId name;
		int precedence; // 1 is lowest precedence.
		EBinaryOperator op;

		OperatorData(OperatorId in_name, int in_precedence, EBinaryOperator in_op)
			: name(in_name)
			, precedence(in_precedence)
			, op(in_op)
		{}
	};

private:
	std::map<OperatorId, OperatorData> operators;

public:
	const std::map<OperatorId, OperatorData>& GetOperators() const
	{
		return operators;
	}

private:
	BinaryOperatorDatabase();
	BinaryOperatorDatabase(BinaryOperatorDatabase const&) = delete;
	BinaryOperatorDatabase& operator=(BinaryOperatorDatabase const&) = delete;
};
