#include "stdafx.h"

#include "potato_common.h"
#include "ast_structure.h"
#include "context.h"

bool CodeGen(ModuleAST* module_root, const std::string& file_name)
{
	if (!Utils::SoftAssert(module_root, "No module root")) return false;

	Context context;
	context.module_ = llvm::make_unique<llvm::Module>(module_root->name_, context.the_context_);
	if (!Utils::SoftAssert(context.module_.get(), "No module created")) return false;
	context.module_->setSourceFileName(file_name);

	module_root->codegen(context);

	context.module_->print(llvm::errs(), nullptr);

	return true;
}

Context::Context()
	: builder_(the_context_)
{}

llvm::AllocaInst* Context::FindVariable(const std::string& name) const
{
	for (auto riter = scope_stack_.rbegin(); riter != scope_stack_.rend(); riter++)
	{
		auto found_it = riter->variables_.find(name);
		if (found_it != riter->variables_.end())
		{
			return found_it->second.alloca_ints_;
		}
	}
	return nullptr;
}

bool Context::AddVariable(VariableHandle variable)
{
	if (!Utils::SoftAssert(!scope_stack_.empty(), "Cannot operate on empty scope"))
	{
		return false;
	}
	auto& var_map = scope_stack_.back().variables_;
	if (var_map.find(variable.name_) != var_map.end())
	{
		Utils::LogError("The variable already exists in the scope:", variable.name_.c_str(), scope_stack_.back().name_.c_str());
		return false;
	}
	var_map.emplace(variable.name_, variable);
	return true;
}