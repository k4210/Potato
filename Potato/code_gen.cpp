#include "stdafx.h"

#include "potato_common.h"
#include "ast_structure.h"
#include "context.h"

bool CodeGen(ModuleAST* module_root, const std::string& file_name)
{
	if (!Utils::SoftAssert(module_root, "No module root")) return false;

	Context context;
	context.module_ = llvm::make_unique<llvm::Module>(module_root->name, context.the_context_);
	if (!Utils::SoftAssert(context.module_.get(), "No module created")) return false;
	context.module_->setSourceFileName(file_name);

	module_root->codegen(context);

	context.module_->print(llvm::errs(), nullptr);

	return true;
}

Context::Context()
	: builder_(the_context_)
{}

VariableData Context::FindVariable(const std::string& name) const
{
	for (auto riter = scope_stack_.rbegin(); riter != scope_stack_.rend(); riter++)
	{
		auto found_it = riter->variables_.find(name);
		if (found_it != riter->variables_.end())
		{
			return found_it->second;
		}
	}
	return VariableData();
}
bool Context::RegisterVariableOnCurrentScope(const VariableData& variable)
{
	if (!Utils::SoftAssert(!scope_stack_.empty(), "Cannot operate on empty scope"))
	{
		return false;
	}
	auto& var_map = scope_stack_.back().variables_;
	if (var_map.find(variable.name) != var_map.end())
	{
		Utils::LogError("The variable already exists in the scope:", variable.name.c_str(), scope_stack_.back().name.c_str());
		return false;
	}
	var_map.emplace(variable.name, variable);
	return true;
}

llvm::AllocaInst* Context::CreateLocalVariable(const VariableData& variable)
{
	assert(function_);
	llvm::Type* type_impl = GetType(variable.type_data);
	
	if (!type_impl)
	{
		Utils::LogError("Unknow type: ", variable.type_data.ToString().c_str());
		return nullptr;
	}
	// why EntryBlock?
	llvm::IRBuilder<> local_builder(&function_->getEntryBlock(), function_->getEntryBlock().begin());
	llvm::AllocaInst* alloca_inst = local_builder.CreateAlloca(type_impl, nullptr, variable.name);
	if (!alloca_inst)
	{
		Utils::LogError("CreateAlloca failed");
		return nullptr;
	}
	RegisterVariableOnCurrentScope(variable);
	return alloca_inst;
}

EVarType Context::GetPotatoDataType(const llvm::Type* type_data)
{
	if (llvm::Type::getInt32Ty(the_context_) == type_data)
	{
		return EVarType::Int;
	}
	else if (llvm::Type::getFloatTy(the_context_) == type_data)
	{
		return EVarType::Float;
	}
	//TODO:

	return EVarType::Void;
}

llvm::Type* Context::GetType(const TypeData& type_data)
{
	switch(type_data.type)
	{
	case EVarType::Void:
		return nullptr;
	case EVarType::Int:
		return llvm::Type::getInt32Ty(the_context_);
	case EVarType::Float:
		return llvm::Type::getFloatTy(the_context_);
	case EVarType::ValueStruct:
		// TODO:
	case EVarType::Reference:
		assert(false);
	}
	return nullptr;
}

llvm::Function* Context::FindFunction(const std::string&, const llvm::Value* optional_owner) const
{
	Utils::SoftAssert(!optional_owner, "Function context not supported yet");

	// we don't support this.. yet

	return nullptr;
}