#pragma once

class Context
{
public:
	struct Scope
	{
		//EFunctionType function_type;
		std::string name;
		//std::string funcion_name;
		std::map<std::string, VariableData> variables_;
	};

	Context();

	llvm::LLVMContext the_context_;
	llvm::IRBuilder<> builder_;
	std::unique_ptr<llvm::Module> module_;

	std::vector<Scope> scope_stack_;
	llvm::Function* function_ = nullptr;

	llvm::Type* GetType(const TypeData& type_data);
	EVarType Context::GetPotatoDataType(const llvm::Type* type_data);

	llvm::Function* FindFunction(const std::string& name, const llvm::Value* optional_owner = nullptr) const;

	llvm::AllocaInst* CreateLocalVariable(const VariableData& variable);
	VariableData FindVariable(const std::string& name) const;
	void OpenScope(const std::string& name)
	{
		scope_stack_.emplace_back(Scope{ name });
	}
	void CloseScope()
	{
		scope_stack_.pop_back();
	}

private:
	bool RegisterVariableOnCurrentScope(const VariableData& variable);

};
