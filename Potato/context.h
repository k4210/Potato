#pragma once

class Context
{
public:

	llvm::LLVMContext the_context_;
	llvm::IRBuilder<> builder_;

	const std::vector<std::shared_ptr<ModuleData>>& already_compiled_modules_;

	struct Scope
	{
		//EFunctionType function_type;
		std::string name;
		//std::string funcion_name;
		std::map<std::string, VariableData> variables_;
	};

	std::shared_ptr<ModuleData> current_module_;
	std::vector<std::shared_ptr<ModuleData>> included_modules_;


	std::shared_ptr<FunctionData> current_function_;
	std::vector<Scope> scope_stack_;

public:
	Context(const std::vector<std::shared_ptr<ModuleData>>& already_compiled_modules);

	//Register error is condition is false. Return the condition.
	void Error(const NodeAST* node_ast, const char* msg0 = "", const char* msg1 = "", const char* msg2 = "") const;
	inline bool Ensure(bool condition, const NodeAST* node_ast, const char* msg0 = "", const char* msg1 = "", const char* msg2 = "") const
	{
		if (!condition)
		{
			Error(node_ast, msg0, msg1, msg2);
		}
		return condition;
	}

	llvm::Type* GetType(const TypeData& type_data);

	std::shared_ptr<FunctionData> FindFunction(const std::string& name, const NodeAST* node_ast, const ExpressionResult* optional_owner) const;

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

	static bool AreTypesCompatible(const TypeData& a, const TypeData& b)
	{
		return a.type == b.type && a.name == b.name;
	}

private:
	template<typename M, M member>
	auto FindEntity(const std::string& name) const
	{
		auto data = Utils::FindByName((*current_module_).*member, name);
		for (unsigned int i = 0; (i < included_modules_.size()) && !data; i++)
		{
			data = Utils::FindByName((*included_modules_[i]).*member, name);
		}
		return data;
	}

	std::shared_ptr<ClassData> FindClassByName(const std::string& name) const
	{
		return FindEntity<decltype(&ModuleData::classes), &ModuleData::classes>(name);
	}

	std::shared_ptr<StructData> FindStructByName(const std::string& name) const
	{
		return FindEntity<decltype(&ModuleData::structures), &ModuleData::structures>(name);
	}

	bool RegisterVariableOnCurrentScope(const VariableData& variable);

};
