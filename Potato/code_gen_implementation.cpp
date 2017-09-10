#include "stdafx.h"

#include "potato_common.h"
#include "ast_expression.h"
#include "ast_flow_control.h"
#include "ast_structure.h"
#include "operator_database.h"
#include "context.h"

llvm::Value* LiteralFloatAST::codegen(Context& context)const 
{
	return llvm::ConstantFP::get(context.the_context_, llvm::APFloat(value_));
}

llvm::Value* LiteralIntegerAST::codegen(Context& context)const
{
	return llvm::ConstantInt::get(context.the_context_, llvm::APInt(sizeof(value_) * 8, value_, true));
}

llvm::Value* LiteralStringAST::codegen(Context& ) const
{
	Utils::SoftAssert(false, "LiteralStringAST::codegen not implemented");
	return nullptr;
}

llvm::Value* LocalVariableDeclarationAST::codegen(Context& context) const 
{
	return context.CreateLocalVariable(variable_);
}

llvm::Value* VariableExprAST::codegen(Context& context)const 
{
	Utils::SoftAssert(!context_, "Variable context not supported yet");

	const VariableData variable_handle = context.FindVariable(name);
	auto alloca_int = llvm::dyn_cast<llvm::AllocaInst>(variable_handle.value);
	return alloca_int
		? context.builder_.CreateLoad(alloca_int, name)
		: nullptr;
}

llvm::Value* UnaryOpAST::codegen(Context& context) const
{ 
	llvm::Value* const value = terminal_ ? terminal_->codegen(context) : nullptr;
	if (!value)
	{
		Utils::LogError("UnaryOpAST::codegen improper value");
		return nullptr;
	}
	const EVarType var_type = context.GetPotatoDataType(value->getType());

	if (opcode_ == EUnaryOperator::Minus)
	{
		if (var_type == EVarType::Float)
		{
			return context.builder_.CreateFNeg(value);
		}
		if (var_type == EVarType::Int)
		{
			return context.builder_.CreateNeg(value);
		}
		Utils::LogError("UnaryOpAST::codegen invalid type for Negation");
		return nullptr;
	}

	if (opcode_ == EUnaryOperator::Negation)
	{
		if (var_type == EVarType::Int)
		{
			return context.builder_.CreateNot(value);
		}
		Utils::LogError("UnaryOpAST::codegen invalid type for Not");
		return nullptr;
	}

	Utils::LogError("UnaryOpAST::codegen unsupported operator");
	return nullptr; 
}

llvm::Value* BinaryOpAST::codegen(Context& context) const
{ 
	const auto operator_data = BinaryOperatorDatabase::Get().GetOperatorData(opcode_);

	llvm::Value* const right_value = rhs_ ? rhs_->codegen(context) : nullptr;
	llvm::Value* const left_value  = lhs_ ? lhs_->codegen(context) : nullptr;
	if (!right_value || !left_value || (operator_data.op != opcode_) || (opcode_ == EBinaryOperator::_Error))
	{
		Utils::LogError("BinaryOpAST::codegen error");
		return nullptr;
	}
	Utils::SoftAssert(operator_data.codegen, "Operator not implemented");

	const EVarType right_var_type = context.GetPotatoDataType(right_value->getType());
	const EVarType left_var_type = context.GetPotatoDataType(left_value->getType());

	const bool valid_types = (right_var_type == left_var_type) && operator_data.supported_data_types.Get(right_var_type);
	if (!valid_types)
	{
		Utils::LogError("BinaryOpAST::codegen invalid types");
		return nullptr;
	}

	llvm::Value* const result = operator_data.codegen(context, left_value, right_value);
	if (!result)
	{
		Utils::LogError("BinaryOpAST::codegen !result");
	}

	return result;
}
llvm::Value* CallExprAST::codegen(Context& context) const
{ 
	llvm::Value* const specified_function_owner = context_ ? context_->codegen(context) : nullptr;
	llvm::Function* const called_function = context.FindFunction(function_name_, specified_function_owner);

	if (!called_function)
	{
		const std::string owner_name = specified_function_owner ? specified_function_owner->getName().str() : "";
		Utils::LogError("CallExprAST::codegen cannot find function:", function_name_.c_str(), owner_name.c_str());
		return nullptr;
	}

	std::vector<llvm::Value *> arguments;
	for (const auto& arg_expr : args_)
	{
		llvm::Value* value = arg_expr->codegen(context);
		arguments.emplace_back(value);
	}

	//IS FUNCTION ACCESSIBLE? 
	//IS MUTABLE?
	//VALIDATE PARAMS

	return context.builder_.CreateCall(called_function, arguments, "calltmp");
}

llvm::Value* TernaryOpAst::codegen(Context&) const { return nullptr; }

void CodeScopeAST::codegen(Context& )const {}
void IfAST::codegen(Context& )const {}
void ForExprAST::codegen(Context& )const {}
void BreakAST::codegen(Context& )const {}
void ContinueAST::codegen(Context& )const {}
void ReturnAST::codegen(Context& )const {}

void FunctionDeclarationAST::codegen(Context& )const {}
void StructureAST::codegen(Context& )const {}
void ClassAST::codegen(Context& )const {}
void ModuleAST::codegen(Context& )const {}
void ImportAST::codegen(Context& )const {}
