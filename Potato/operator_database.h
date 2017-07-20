#pragma once

#include <map>
#include "potato_common.h"

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

class UnaryOperatorDatabase
{
public:
	static const UnaryOperatorDatabase& Get();

	struct OperatorData
	{
		OperatorId name;
		EUnaryOperator op;

		OperatorData(OperatorId in_name, EUnaryOperator in_op)
			: name(in_name)
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

	OperatorData FindOperator(OperatorId key) const
	{
		auto found_iter = operators.find(key);
		return (found_iter != operators.end()) ? found_iter->second : OperatorData(nullptr, EUnaryOperator::_Error);
	}

private:
	UnaryOperatorDatabase();
	UnaryOperatorDatabase(UnaryOperatorDatabase const&) = delete;
	UnaryOperatorDatabase& operator=(UnaryOperatorDatabase const&) = delete;
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

	OperatorData FindOperator(OperatorId key) const
	{
		auto found_iter = operators.find(key);
		return (found_iter != operators.end()) ? found_iter->second : OperatorData(nullptr, 0, EBinaryOperator::_Error);
	}

private:
	BinaryOperatorDatabase();
	BinaryOperatorDatabase(BinaryOperatorDatabase const&) = delete;
	BinaryOperatorDatabase& operator=(BinaryOperatorDatabase const&) = delete;
};


