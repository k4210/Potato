#pragma once

struct VariableHandle
{
	std::string name_;
	TypeData type_data_;
	llvm::AllocaInst* alloca_ints_;
};

struct Scope
{
	//EFunctionType function_type;
	std::string name_;
	//std::string funcion_name;
	std::map<std::string, VariableHandle> variables_;
};

class Context
{
public:
	Context();

	llvm::LLVMContext the_context_;
	llvm::IRBuilder<> builder_;
	std::unique_ptr<llvm::Module> module_;

	std::vector<Scope> scope_stack_;
	llvm::Function* function_ = nullptr;

	llvm::AllocaInst* FindVariable(const std::string& name) const;
	bool AddVariable(VariableHandle variable);
	void OpenScope(const std::string& name)
	{
		scope_stack_.emplace_back(Scope{ name });
	}
	void CloseScope()
	{
		scope_stack_.pop_back();
	}
	//class data

	//function data

	//scope data
	// std::map<std::string, llvm::AllocaInst *> NamedValues;
};
