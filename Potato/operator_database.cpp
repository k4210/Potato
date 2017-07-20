#include "operator_database.h"
#include "utils.h"
#include <memory>

OperatorId::OperatorId(const char* in_name)
{
	if (strlen(in_name) >= kNameMaxSize)
	{
		Utils::LogError("Too long operator ID:", in_name);
		in_name = "err";
	}
	strcpy_s<kNameMaxSize>(name, in_name);
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
		operators.emplace(id, OperatorData(id, in_op));
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
		operators.emplace(id, OperatorData(id, in_precedence, in_op));
	};
	add("=", 5, EBinaryOperator::Assign);
	add("=?", 5, EBinaryOperator::AssignIfValidChain);
	// ternary
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
	//unary
}
