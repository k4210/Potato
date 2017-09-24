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
		context.Error(this, "UnaryOpAST::codegen invalid type for Negation");
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

	context.Error(this, "UnaryOpAST::codegen unsupported operator");
	return nullptr; 
}

llvm::Value* BinaryOpAST::codegen(Context& context) const
{ 
	const auto operator_data = BinaryOperatorDatabase::Get().GetOperatorData(opcode_);

	llvm::Value* const right_value = rhs_ ? rhs_->codegen(context) : nullptr;
	llvm::Value* const left_value  = lhs_ ? lhs_->codegen(context) : nullptr;
	if (!right_value || !left_value || (operator_data.op != opcode_) || (opcode_ == EBinaryOperator::_Error))
	{
		context.Error(this, "BinaryOpAST::codegen error");
		return nullptr;
	}
	Utils::SoftAssert(operator_data.codegen, "Operator not implemented");

	const EVarType right_var_type = context.GetPotatoDataType(right_value->getType());
	const EVarType left_var_type = context.GetPotatoDataType(left_value->getType());

	const bool valid_types = (right_var_type == left_var_type) && operator_data.supported_data_types.Get(right_var_type);
	if (!valid_types)
	{
		context.Error(this, "BinaryOpAST::codegen invalid types");
		return nullptr;
	}

	llvm::Value* const result = operator_data.codegen(context, left_value, right_value);
	if (!result)
	{
		context.Error(this, "BinaryOpAST::codegen !result");
	}

	return result;
}

llvm::Value* CallExprAST::codegen(Context& context) const
{ 
	llvm::Value* const specified_function_owner = context_ ? context_->codegen(context) : nullptr;
	const auto called_function = context.FindFunction(function_name_, specified_function_owner);
	if (!called_function || !called_function->function)
	{
		const std::string owner_name = specified_function_owner ? specified_function_owner->getName().str() : "";
		context.Error(this, "CallExprAST::codegen cannot find function:", function_name_.c_str(), owner_name.c_str());
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

	return context.builder_.CreateCall(called_function->function, arguments, "calltmp");
}

llvm::Value* TernaryOpAst::codegen(Context&) const { return nullptr; }

void CodeScopeAST::codegen(Context& context)const 
{
	context.OpenScope(nullptr);
	for (const auto& child : code_)
	{
		NodeAST* ast_node = child.get();
		if (auto control_flow = dynamic_cast<ControlFlowAST*>(ast_node))
		{
			control_flow->codegen(context);
		}
		else if (auto expression = dynamic_cast<ExprAST*>(ast_node))
		{
			expression->codegen(context);
		}
		else
		{
			Utils::SoftAssert(false, "wrong ast node");
		}
	}
}
void IfAST::codegen(Context& )const {}
void ForExprAST::codegen(Context& )const {}
void BreakAST::codegen(Context& )const {}
void ContinueAST::codegen(Context& )const {}
void ReturnAST::codegen(Context& )const {}

void FunctionDeclarationAST::codegen(Context& )const 
{
	
}

void StructureAST::codegen(Context& )const 
{

}

void ClassAST::codegen(Context& )const 
{

}

void ModuleAST::codegen(Context& context) const
{
	if (!context.Ensure( module_data_.get(), this, "no module")) return;
	if (!context.Ensure( !module_data_->module_implementation, this, "module already compiled")) return;
	if (!context.Ensure( !context.current_module_.get(), this, "unexpected")) return;
	if (!context.Ensure( !Utils::Contains(context.already_compiled_modules_, module_data_), this, "module already compiled 2")) return;
	for (const auto& module : context.already_compiled_modules_)
	{
		if(!context.Ensure(module->name != module_data_->name, this, "module name is not unique")) return;
	}

	module_data_->module_implementation = llvm::make_unique<llvm::Module>(module_data_->name, context.the_context_);
	if (!context.Ensure(module_data_->module_implementation != nullptr, this, "module implementation not created")) return;
	
	context.current_module_ = module_data_;
	for (auto& item : imports_)
	{
		item->codegen(context);
	}
	for (auto& item : structures_)
	{
		item->codegen(context);
	}
	for (auto& item : classes_)
	{
		item->codegen(context);// Split?
	}
	for (auto& item : functions_)
	{
		item->codegen(context);
	}
	context.current_module_ = nullptr;
}

void ImportAST::codegen(Context& context)const 
{
	for (auto& module : context.already_compiled_modules_)
	{
		if (module->name == name)
		{
			context.current_module_->imported_modules.emplace_back(module);
		}
	}
}
