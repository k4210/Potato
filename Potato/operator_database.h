#pragma once

#include "potato_common.h"

struct OperatorId
{
	static const int kNameMaxSize = 4;
	unsigned int data;
	static_assert(sizeof(unsigned int) == kNameMaxSize * sizeof(char), "wrong size of OperatorId");
private:
	char* name()
	{
		return reinterpret_cast<char*>(&data);
	}
	const char* name() const
	{
		return reinterpret_cast<const char*>(&data);
	}
public:
	const char* c_str() const
	{
		return name();
	}
	operator const char*() const 
	{ 
		return name();
	}
	OperatorId(const char* in_name);

	bool operator<(const OperatorId& other) const
	{
		return data < other.data;
	}
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
	std::map<EUnaryOperator, OperatorData> operators_map;
	std::map<OperatorId, EUnaryOperator> id_to_op;
public:
	const std::map<EUnaryOperator, OperatorData>& GetOperators() const
	{
		return operators_map;
	}

	OperatorData GetOperatorData(EUnaryOperator op) const
	{
		const auto found_data = operators_map.find(op);
		return (found_data != operators_map.end()) ? found_data->second : OperatorData(nullptr, EUnaryOperator::_Error);
	}

	OperatorData FindOperator(OperatorId key) const
	{
		const auto found_op = id_to_op.find(key);
		const EUnaryOperator op = (found_op != id_to_op.end()) ? found_op->second : EUnaryOperator::_Error;
		return GetOperatorData(op);
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
	std::map<EBinaryOperator, OperatorData> operators_map;
	std::map<OperatorId, EBinaryOperator> id_to_op;
	std::vector<EBinaryOperator> op_sorted_by_precedence_descending;
public:
	const std::map<EBinaryOperator, OperatorData>& GetOperators() const
	{
		return operators_map;
	}

	OperatorData GetOperatorData(EBinaryOperator op) const
	{
		const auto found_data = operators_map.find(op);
		return (found_data != operators_map.end()) ? found_data->second : OperatorData(nullptr, 0, EBinaryOperator::_Error);
	}

	OperatorData FindOperator(OperatorId key) const
	{
		const auto found_op = id_to_op.find(key);
		const EBinaryOperator op = (found_op != id_to_op.end()) ? found_op->second : EBinaryOperator::_Error;
		return GetOperatorData(op);
	}

	OperatorData GetHighestPrecedenceOp() const
	{
		return GetOperatorData(op_sorted_by_precedence_descending[0]);
	}
	
	OperatorData GetNextOpWithLowerPrecedence(EBinaryOperator op) const;

private:
	BinaryOperatorDatabase();
	BinaryOperatorDatabase(BinaryOperatorDatabase const&) = delete;
	BinaryOperatorDatabase& operator=(BinaryOperatorDatabase const&) = delete;
};


