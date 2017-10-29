#include "stdafx.h"
#include "operator_database.h"
#include "context.h"
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
		operators_map_.emplace(in_op, OperatorData(id, in_op));
		id_to_op_.emplace(id, in_op);
	};

	add("-", EUnaryOperator::Minus);
	add("!", EUnaryOperator::Negation);
	add("~", EUnaryOperator::ByteNegation);
}

BinaryOperatorDatabase::BinaryOperatorDatabase()
{
	auto add = [&](const char* in_name
		, int in_precedence
		, EBinaryOperator in_op
		, Flag32<EVarType> types
		, CodegenFunction Codegen)
	{
		const OperatorId id(in_name);
		operators_map_.emplace(in_op, OperatorData(id, in_precedence, in_op, types, Codegen));
		id_to_op_.emplace(id, in_op);
		op_sorted_by_precedence_descending_.push_back(in_op);
	};
	add("=", 5, EBinaryOperator::Assign, EVarType::Int | EVarType::Float | EVarType::ValueStruct, nullptr);
	add("=?", 5, EBinaryOperator::AssignIfValidChain, EVarType::Int | EVarType::Float | EVarType::ValueStruct, nullptr);

	add("+=", 5, EBinaryOperator::AddAssign, EVarType::Float | EVarType::Int, nullptr);
	add("-=", 5, EBinaryOperator::SubAssign, EVarType::Float | EVarType::Int, nullptr);
	add("*=", 5, EBinaryOperator::MultiplyAssign, EVarType::Float | EVarType::Int, nullptr);
	add("/=", 5, EBinaryOperator::DivideAssign, EVarType::Float | EVarType::Int, nullptr);

	add("%=", 5, EBinaryOperator::ModuloAssign, EVarType::Int, nullptr);
	add("&=", 5, EBinaryOperator::AndAssign, EVarType::Int, nullptr);
	add("|=", 5, EBinaryOperator::OrAssign, EVarType::Int, nullptr);
	///////////////////////////////
	add("|", 10, EBinaryOperator::Or, EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return { TypeData(EVarType::Int), context.builder_.CreateOr(lhs, rhs) };
	});
	add("&", 15, EBinaryOperator::And, EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return { TypeData(EVarType::Int), context.builder_.CreateAnd(lhs, rhs) };
	});
	///////////////////////////////
	add("==", 20, EBinaryOperator::Equal, EVarType::Float | EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return ExpressionResult{ TypeData(EVarType::Int), (EVarType::Int == lhs.type_data.type)
			? context.builder_.CreateICmpEQ(lhs, rhs) 
			: context.builder_.CreateFCmpOEQ(lhs, rhs) };
	});
	add("!=", 20, EBinaryOperator::NotEqual, EVarType::Float | EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return ExpressionResult{ TypeData(EVarType::Int), (EVarType::Int == lhs.type_data.type)
			? context.builder_.CreateICmpNE(lhs, rhs)
			: context.builder_.CreateFCmpONE(lhs, rhs) };
	});
	add(">=", 25, EBinaryOperator::GreaterOrEqual, EVarType::Float | EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return ExpressionResult{ TypeData(EVarType::Int), (EVarType::Int == lhs.type_data.type)
			? context.builder_.CreateICmpSGE(lhs, rhs)
			: context.builder_.CreateFCmpOGE(lhs, rhs) };
	});
	add("<=", 25, EBinaryOperator::LessOrEqual, EVarType::Float | EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return ExpressionResult{ TypeData(EVarType::Int), (EVarType::Int == lhs.type_data.type)
			? context.builder_.CreateICmpSLE(lhs, rhs)
			: context.builder_.CreateFCmpOLE(lhs, rhs) };
	});
	add("<", 25, EBinaryOperator::Less, EVarType::Float | EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return ExpressionResult{ TypeData(EVarType::Int), (EVarType::Int == lhs.type_data.type)
			? context.builder_.CreateICmpSLT(lhs, rhs)
			: context.builder_.CreateFCmpOLT(lhs, rhs) };
	});
	add(">", 25, EBinaryOperator::Greater, EVarType::Float | EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return ExpressionResult{ TypeData(EVarType::Int), (EVarType::Int == lhs.type_data.type)
			? context.builder_.CreateICmpSGT(lhs, rhs)
			: context.builder_.CreateFCmpOGT(lhs, rhs) };
	});
	///////////////////////////////
	add("+", 30, EBinaryOperator::Add, EVarType::Float | EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return ExpressionResult{ lhs.type_data, (EVarType::Int == lhs.type_data.type)
			? context.builder_.CreateAdd(lhs, rhs)
			: context.builder_.CreateFAdd(lhs, rhs) };
	});
	add("-", 30, EBinaryOperator::Sub, EVarType::Float | EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return ExpressionResult{ lhs.type_data, (EVarType::Int == lhs.type_data.type)
			? context.builder_.CreateSub(lhs, rhs)
			: context.builder_.CreateFSub(lhs, rhs) };
	});
	add("*", 35, EBinaryOperator::Multiply, EVarType::Float | EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return ExpressionResult{ lhs.type_data, (EVarType::Int == lhs.type_data.type)
			? context.builder_.CreateMul(lhs, rhs)
			: context.builder_.CreateFMul(lhs, rhs) };;
	});
	add("/", 35, EBinaryOperator::Divide, EVarType::Float | EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return ExpressionResult{ lhs.type_data, (EVarType::Int == lhs.type_data.type)
			? context.builder_.CreateSDiv(lhs, rhs)
			: context.builder_.CreateFDiv(lhs, rhs) };
	});
	///////////////////////////////
	add("%", 35, EBinaryOperator::Modulo, EVarType::Int, [](Context& context, ExpressionResult lhs, ExpressionResult rhs) -> ExpressionResult
	{
		return ExpressionResult{ TypeData(EVarType::Int), context.builder_.CreateSRem(lhs, rhs) };
	});

	std::reverse(op_sorted_by_precedence_descending_.begin(), op_sorted_by_precedence_descending_.end());
}

BinaryOperatorDatabase::OperatorData BinaryOperatorDatabase::GetNextOpWithLowerPrecedence(EBinaryOperator op) const
{
	auto found_iter = std::find(op_sorted_by_precedence_descending_.begin(),op_sorted_by_precedence_descending_.end(),op);
	if (found_iter != op_sorted_by_precedence_descending_.end())
	{
		const int current_precedence = GetOperatorData(op).precedence;
		for (++found_iter; found_iter != op_sorted_by_precedence_descending_.end(); ++found_iter)
		{
			const OperatorData data = GetOperatorData(*found_iter);
			if (current_precedence > data.precedence)
			{
				return data;
			}
		}
	}
	return OperatorData();

}