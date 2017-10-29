#include "stdafx.h"

#include "potato_common.h"
#include "ast_structure.h"
#include "context.h"

bool CodeGen(ModuleAST* module_root, const std::vector<std::shared_ptr<ModuleData>>& already_compiled_modules, const std::string& file_name)
{
	if (!Utils::SoftAssert(module_root && module_root->module_data_, "No module root"))
	{
		return false;
	}

	Context context(already_compiled_modules);
	module_root->Codegen(context);

	if (!module_root->module_data_->module_implementation)
	{
		return false;
	}

	module_root->module_data_->module_implementation->setSourceFileName(file_name);
	module_root->module_data_->module_implementation->print(llvm::errs(), nullptr);
	return true;
}

Context::Context(const std::vector<std::shared_ptr<ModuleData>>& already_compiled_modules)
	: builder_(the_context_)
	, already_compiled_modules_(already_compiled_modules)
{}

void Context::Error(const NodeAST*, const char* msg0, const char* msg1, const char* msg2) const
{
	Utils::LogError(msg0, msg1, msg2);
}

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
	assert(current_function_ && current_function_->function);
	llvm::Type* type_impl = GetType(variable.type_data);
	
	if (!type_impl)
	{
		Utils::LogError("Unknow type: ", variable.type_data.ToString().c_str());
		return nullptr;
	}
	// why EntryBlock?
	llvm::IRBuilder<> local_builder(&current_function_->function->getEntryBlock(), current_function_->function->getEntryBlock().begin());
	llvm::AllocaInst* alloca_inst = local_builder.CreateAlloca(type_impl, nullptr, variable.name);
	if (!alloca_inst)
	{
		Utils::LogError("CreateAlloca failed");
		return nullptr;
	}
	RegisterVariableOnCurrentScope(variable);
	return alloca_inst;
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
		{
			auto data = FindEntity<decltype(&ModuleData::structures), &ModuleData::structures>(type_data.name);
			Ensure(data && data->type, nullptr, "Struct '", type_data.name.c_str(), "' is found, but it's llvm type is unknown");
			return data ? data->type : nullptr;
		} 
	case EVarType::Reference:
		{
			auto data = FindEntity<decltype(&ModuleData::classes), &ModuleData::classes>(type_data.name);
			Ensure(data && data->type, nullptr, "Class '", type_data.name.c_str(), "' is found, but it's llvm type is unknown");
			return data ? data->type : nullptr;
		}
	}
	return nullptr;
}

std::shared_ptr<FunctionData> Context::FindFunction(const std::string& name, const ExpressionResult* optional_owner) const
{
	if (!current_function_)
	{
		Error(nullptr, "no function");
		return nullptr;
	}

	if (optional_owner)
	{

	}
	else
	{
		auto current_function_owner = current_function_->owner.lock();
		auto current_class = dynamic_cast<ClassData*>(current_function_owner.get()); //only when function is not static ?
		if (auto func = current_class ? Utils::FindByName(current_class->functions, name) : nullptr)
		{
			return std::shared_ptr<FunctionData>(func);
		}
		if (auto func = Utils::FindByName(current_module_->functions, name))
		{
			return std::shared_ptr<FunctionData>(func);
		}
		for (auto& module : included_modules_)
		{
			if (auto func = module ? Utils::FindByName(module->functions, name) : nullptr)
			{
				return std::shared_ptr<FunctionData>(func);
			}
		}
	}

	return nullptr;
}