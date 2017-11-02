#include "stdafx.h"

#include "potato_common.h"
#include "ast_expression.h"
#include "ast_flow_control.h"
#include "ast_structure.h"
#include "operator_database.h"
#include "context.h"

ExpressionResult LiteralFloatAST::Codegen(Context& context) const
{
	return { 
		TypeData(EVarType::Float),
		llvm::ConstantFP::get(context.the_context_, llvm::APFloat(value_))
	};
}

ExpressionResult LiteralIntegerAST::Codegen(Context& context)const
{
	return { 
		TypeData(EVarType::Int),
		llvm::ConstantInt::get(context.the_context_, llvm::APInt(sizeof(value_) * 8, value_, true))
	};
}

ExpressionResult LiteralStringAST::Codegen(Context& ) const
{
	Utils::SoftAssert(false, "LiteralStringAST::codegen not implemented");
	return {};
}

ExpressionResult LocalVariableDeclarationAST::Codegen(Context& context) const
{
	return {
		variable_.type_data,
		context.CreateLocalVariable(variable_)
	};
}

ExpressionResult VariableExprAST::Codegen(Context& context)const
{
	Utils::SoftAssert(!context_, "Variable context not supported yet");

	const VariableData variable_handle = context.FindVariable(name);
	auto alloca_inst = llvm::dyn_cast<llvm::AllocaInst>(variable_handle.value);
	return {
		variable_handle.type_data,
		alloca_inst ? context.builder_.CreateLoad(alloca_inst, name) : nullptr
	};
}

ExpressionResult UnaryOpAST::Codegen(Context& context) const
{ 
	auto input_epression = terminal_ ? terminal_->Codegen(context) : ExpressionResult();
	if (!input_epression.value)
	{
		context.Error(this, "UnaryOpAST::codegen improper value");
		return {};
	}

	if (opcode_ == EUnaryOperator::Minus)
	{
		if (input_epression.type_data.type == EVarType::Float)
		{
			return { TypeData(EVarType::Float), context.builder_.CreateFNeg(input_epression.value) };
		}
		if (input_epression.type_data.type == EVarType::Int)
		{
			return { TypeData(EVarType::Int), context.builder_.CreateNeg(input_epression.value) };
		}
		context.Error(this, "UnaryOpAST::codegen invalid type for Negation");
		return {};
	}

	if (opcode_ == EUnaryOperator::Negation)
	{
		if (input_epression.type_data.type == EVarType::Int)
		{
			return { TypeData(EVarType::Int), context.builder_.CreateNot(input_epression.value) };
		}
		context.Error(this, "UnaryOpAST::codegen invalid type for Not");
		return {};
	}

	context.Error(this, "UnaryOpAST::codegen unsupported operator");
	return {};
}

ExpressionResult BinaryOpAST::Codegen(Context& context) const
{ 
	const auto operator_data = BinaryOperatorDatabase::Get().GetOperatorData(opcode_);

	const auto right_value = rhs_ ? rhs_->Codegen(context) : ExpressionResult{};
	const auto left_value  = lhs_ ? lhs_->Codegen(context) : ExpressionResult{};
	if (!right_value || !left_value || (operator_data.op != opcode_) || (opcode_ == EBinaryOperator::_Error))
	{
		context.Error(this, "BinaryOpAST::codegen error");
		return {};
	}
	Utils::SoftAssert(operator_data.Codegen, "Operator not implemented");

	const bool compatible_types = Context::AreTypesCompatible(right_value.type_data, left_value.type_data);
	const bool valid_types = compatible_types && operator_data.supported_data_types.Get(right_value.type_data.type);
	if (!valid_types)
	{
		context.Error(this, "BinaryOpAST::codegen invalid types");
		return {};
	}

	const auto result = operator_data.Codegen(context, left_value, right_value);
	if (!result)
	{
		context.Error(this, "BinaryOpAST::codegen !result");
	}

	return result;
}

ExpressionResult CallExprAST::Codegen(Context& context) const
{ 
	const auto specified_function_owner = context_ ? context_->Codegen(context) : ExpressionResult{};
	const auto called_function = context.FindFunction(function_name_, this, context_ ? &specified_function_owner : nullptr);
	if (!called_function || !called_function->function)
	{
		context.Error(this
			, "CallExprAST::codegen cannot find function:"
			, function_name_.c_str()
			, specified_function_owner.type_data.name.c_str());
		return {};
	}

	std::vector<llvm::Value *> arguments;
	for (const auto& arg_expr : args_)
	{
		llvm::Value* value = arg_expr->Codegen(context);
		arguments.emplace_back(value);
	}

	//IS FUNCTION ACCESSIBLE? 
	//IS MUTABLE?
	//VALIDATE PARAMS

	auto llvm_func = context.builder_.CreateCall(called_function->function, arguments, "calltmp");
	return { called_function->return_type, llvm_func };
}

ExpressionResult TernaryOpAst::Codegen(Context&) const { return {}; }

void CodeScopeAST::Codegen(Context& context)const 
{
	context.OpenScope(nullptr);
	for (const auto& child : code_)
	{
		NodeAST* ast_node = child.get();
		if (auto control_flow = dynamic_cast<ControlFlowAST*>(ast_node))
		{
			control_flow->Codegen(context);
		}
		else if (auto expression = dynamic_cast<ExprAST*>(ast_node))
		{
			expression->Codegen(context);
		}
		else
		{
			Utils::SoftAssert(false, "wrong ast node");
		}
	}
}
void IfAST::Codegen(Context& )const 
{

}

void ForExprAST::Codegen(Context& )const 
{

}

void BreakAST::Codegen(Context& )const 
{

}

void ContinueAST::Codegen(Context& )const 
{

}

void ReturnAST::Codegen(Context& )const 
{

}

void FunctionDeclarationAST::Codegen(Context& ) const 
{
	
}

void FunctionDeclarationAST::RegisterFunction(Context& context, llvm::StructType* implicit_owner_arg) const
{
	Utils::SoftAssert(function_data_ && !function_data_->function);
	const auto& parameters = function_data_->parameters;

	const unsigned int arg_offset = implicit_owner_arg ? 1 : 0;
	std::vector<llvm::Type*> arguments(parameters.size() + arg_offset, nullptr);
	if (implicit_owner_arg)
	{
		arguments[0] = implicit_owner_arg;
	}
	std::transform(parameters.begin(), parameters.end(), arguments.begin() + arg_offset, [&](const VariableData& variable_data)
	{
		return context.GetType(variable_data.type_data);
	});
	llvm::FunctionType* function_type = llvm::FunctionType::get(context.GetType(function_data_->return_type), arguments, false);

	function_data_->function = llvm::Function::Create(function_type, 
		llvm::Function::ExternalLinkage, 
		function_data_->name, 
		context.current_module_->module_implementation.get());
	
	unsigned int idx = 0;
	for (auto &arg : function_data_->function->args())
	{
		if (implicit_owner_arg && 0 == idx)
		{
			arg.setName("this");
		}
		else
		{
			arg.setName(function_data_->parameters[arg_offset + idx].name);
		}
	}
}

void StructureAST::RegisterType(Context& context) const
{
	context.Ensure(nullptr == structure_data_->type, this);
	structure_data_->type = llvm::StructType::create(context.the_context_, structure_data_->name);
	context.Ensure(nullptr != structure_data_->type, this);
}

void StructureAST::GenerateDataLayout(Context& context) const
{
	std::vector<llvm::Type*> members;
	for (auto& member_iter : structure_data_->member_fields)
	{
		//context.Ensure(nullptr == member_iter.second.value, this);
		// TODO: add llvm::Type to TypeData?
		members.push_back(context.GetType(member_iter.second.type_data));
	}
	
	structure_data_->type->setBody(members);
}

void StructureAST::RegisterFunctions(Context& context) const
{
	for (auto& function_ast : functions_)
	{
		function_ast->RegisterFunction(context, structure_data_->type);
	}
}

void StructureAST::Codegen(Context& context) const
{
	for (auto& function_ast : functions_)
	{
		function_ast->Codegen(context);
	}
}

void ModuleAST::Codegen(Context& context) const
{
	if (!context.Ensure( module_data_.get(), this, "no module")) return;
	if (!context.Ensure( !module_data_->module_implementation, this, "module already compiled")) return;
	if (!context.Ensure( !context.current_module_.get(), this, "unexpected")) return;
	if (!context.Ensure( !Utils::Contains(context.already_compiled_modules_, module_data_), this, "module already compiled 2")) return;
	if (!context.Ensure( !Utils::FindByName(context.already_compiled_modules_, module_data_->name), this, "module name is not unique")) return;

	module_data_->module_implementation = llvm::make_unique<llvm::Module>(module_data_->name, context.the_context_);
	if (!context.Ensure(module_data_->module_implementation != nullptr, this, "module implementation not created")) return;
	
	context.current_module_ = module_data_;
	for (auto& item : imports_)		item->Import(context);
	
	//At this point all HighLevelEntity objects are filled, and only the llvm implementation is missing.

	for (auto& item : structures_)	item->RegisterType(context);
	for (auto& item : classes_)		item->RegisterType(context);

	//TODO: make sure, that types in all imported modules are registered

	for (auto& item : structures_)	item->GenerateDataLayout(context);
	for (auto& item : classes_)		item->GenerateDataLayout(context);

	for (auto& item : structures_)	item->RegisterFunctions(context);
	for (auto& item : classes_)		item->RegisterFunctions(context);
	for (auto& item : functions_)	item->RegisterFunction(context, nullptr);

	for (auto& item : structures_)	item->Codegen(context);
	for (auto& item : classes_)		item->Codegen(context);
	for (auto& item : functions_)	item->Codegen(context);

	context.current_module_ = nullptr;
	context.included_modules_.clear();
}

void ImportAST::Import(Context& context) const
{
	auto compiled_module = Utils::FindByName(context.already_compiled_modules_, name);
	if (compiled_module)
	{
		const bool already_included = Utils::ContainsIf(context.current_module_->imported_modules
			, [&](auto ptr) { return compiled_module == ptr.lock(); });
		context.Ensure(!already_included, this, "module already imported");

		context.current_module_->imported_modules.emplace_back(compiled_module);
		context.included_modules_.emplace_back(compiled_module);
	}
}
