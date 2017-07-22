#include "stdafx.h"
#include "operator_database.h"
#include "utils.h"

OperatorId::OperatorId(const char* in_name)
{
	if (nullptr == in_name)
	{
		in_name = "";
	}
	else if (strlen(in_name) >= kNameMaxSize)
	{
		Utils::LogError("Wrong operator ID:", in_name);
		in_name = "err";
	}
	const auto sizeof_str = strlen(in_name);
	strcpy_s(name(), kNameMaxSize, in_name);
	memset(name() + sizeof_str, 0, (kNameMaxSize - sizeof_str) * sizeof(char));
}

const BinaryOperatorDatabase& BinaryOperatorDatabase::Get()
{
	static BinaryOperatorDatabase instance;
	return instance;
}

const UnaryOperatorDatabase& UnaryOperatorDatabase::Get()
{
	static UnaryOperatorDatabase instance;
	return instance;
}

UnaryOperatorDatabase::UnaryOperatorDatabase()
{
	auto add = [&](const char* in_name, EUnaryOperator in_op)
	{
		const OperatorId id(in_name);
		operators_map.emplace(in_op, OperatorData(id, in_op));
		id_to_op.emplace(id, in_op);
	};

	add("-", EUnaryOperator::Minus);
	add("!", EUnaryOperator::Negation);
	add("`", EUnaryOperator::ByteNegation);
}

BinaryOperatorDatabase::BinaryOperatorDatabase()
{
	auto add = [&](const char* in_name, int in_precedence, EBinaryOperator in_op)
	{
		const OperatorId id(in_name);
		operators_map.emplace(in_op, OperatorData(id, in_precedence, in_op));
		id_to_op.emplace(id, in_op);
		op_sorted_by_precedence_descending.push_back(in_op);
	};
	add("=", 5, EBinaryOperator::Assign);
	add("=?", 5, EBinaryOperator::AssignIfValidChain);
	add("+=", 5, EBinaryOperator::AddAssign);
	add("-=", 5, EBinaryOperator::SubAssign);
	add("*=", 5, EBinaryOperator::MultiplyAssign);
	add("/=", 5, EBinaryOperator::DivideAssign);
	add("%=", 5, EBinaryOperator::ModuloAssign);
	add("&=", 5, EBinaryOperator::AndAssign);
	add("|=", 5, EBinaryOperator::OrAssign);
	add("|", 10, EBinaryOperator::Or);
	add("&", 15, EBinaryOperator::And);
	add("==", 20, EBinaryOperator::Equal);
	add("!=", 20, EBinaryOperator::NotEqual);
	add(">=", 25, EBinaryOperator::GreaterOrEqual);
	add("<=", 25, EBinaryOperator::LessOrEqual);
	add("<", 25, EBinaryOperator::Less);
	add(">", 25, EBinaryOperator::Greater);
	add("+", 30, EBinaryOperator::Add);
	add("-", 30, EBinaryOperator::Sub);
	add("*", 35, EBinaryOperator::Multiply);
	add("/", 35, EBinaryOperator::Divide);
	add("%", 35, EBinaryOperator::Modulo);

	std::reverse(op_sorted_by_precedence_descending.begin(), op_sorted_by_precedence_descending.end());
}

BinaryOperatorDatabase::OperatorData BinaryOperatorDatabase::GetNextOpWithLowerPrecedence(EBinaryOperator op) const
{
	auto found_iter = std::find(op_sorted_by_precedence_descending.begin(),op_sorted_by_precedence_descending.end(),op);
	if (found_iter != op_sorted_by_precedence_descending.end())
	{
		const int current_precedence = GetOperatorData(op).precedence;
		for (++found_iter; found_iter != op_sorted_by_precedence_descending.end(); ++found_iter)
		{
			const OperatorData data = GetOperatorData(*found_iter);
			if (current_precedence > data.precedence)
			{
				return data;
			}
		}
	}
	return OperatorData(nullptr, 0, EBinaryOperator::_Error);

}