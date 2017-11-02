#pragma once

#include "potato_common.h"
#include "ast_expression.h"
#include "ast_flow_control.h"
#include "utils.h"

class HighLevelAST : public NodeAST
{
};

class FunctionDeclarationAST : public HighLevelAST
{
public:
	std::shared_ptr<FunctionData> function_data_;
	std::unique_ptr<CodeScopeAST> optional_body_;

	FunctionDeclarationAST() = default;
	FunctionDeclarationAST(std::shared_ptr<FunctionData> function_data)
		: function_data_(function_data)
	{}

	void log(Logger& logger, const char* contect_str) const override
	{
		Utils::SoftAssert(function_data_.get(), "error..");
		std::string result = "FunctionDeclarationAST ";
		result += function_data_->name;
		logger.PrintLine(contect_str, result.c_str());
		logger.IncreaseIndent();
		logger.PrintLine("return", function_data_->return_type.ToString().c_str());
		for (auto& param : function_data_->parameters)
		{
			logger.PrintLine("parameter", param.ToString().c_str());
		}
		if (optional_body_)
		{
			optional_body_->log(logger, "body");
		}
		logger.DecreaseIndent();
	}
	void RegisterFunction(Context& context, llvm::StructType* implicit_owner_arg) const;
	void Codegen(Context& context) const;
};

class StructureAST : public HighLevelAST
{
public:
	//Todo: add info - expose to other modules? yes
	std::shared_ptr<StructData> structure_data_;
	std::vector<std::unique_ptr<FunctionDeclarationAST>> functions_;

	StructureAST() = default;
	StructureAST(std::shared_ptr<StructData> structure_data)
		: structure_data_(structure_data)
	{}

	virtual const char* GetName() const 
	{
		return "StructureAST ";
	}

	void log(Logger& logger, const char* contect_str) const override
	{
		Utils::SoftAssert(structure_data_.get(), "error..");
		std::string result( GetName() );
		result += structure_data_->name;
		logger.PrintLine(contect_str, result.c_str());

		logger.IncreaseIndent();
		for (auto& var : structure_data_->member_fields)
		{
			logger.PrintLine("member_field", var.first.c_str());
		}

		for (auto& func : functions_)
		{
			func->log(logger, "func");
		}
		logger.DecreaseIndent();
	}
	void RegisterType(Context& context) const;
	void GenerateDataLayout(Context& context) const;
	void RegisterFunctions(Context& context) const;
	void Codegen(Context& context) const;

	void BindParsedChildren()
	{
		for (auto& func_ast : functions_)
		{
			if (func_ast && func_ast->function_data_)
			{
				func_ast->function_data_->owner = structure_data_;
				structure_data_->functions.emplace(func_ast->function_data_->name, func_ast->function_data_);
			}
		}
	}
};

class ClassAST : public StructureAST
{
public:
	

	//TODO: interfaces

	ClassAST() = default;
	ClassAST(std::shared_ptr<ClassData> class_data)
		: StructureAST(class_data)
	{}

	const ClassData* GetClassData() const
	{
		return static_cast<const ClassData*>(structure_data_.get());
	}
	ClassData* GetClassData()
	{
		return static_cast<ClassData*>(structure_data_.get());
	}

	const char* GetName() const override
	{
		return "ClassAST ";
	}

	void log(Logger& logger, const char* contect_str) const override
	{
		StructureAST::log(logger, contect_str);
		logger.PrintLine(contect_str, "Base class: ", GetClassData()->base_class.c_str());
	}
};

class ImportAST : public HighLevelAST
{
public:
	std::string name;

	ImportAST(const std::string& in_name)
		: name(in_name)
	{}
	void log(Logger& logger, const char* contect_str) const override
	{
		std::string result = "ImportAST ";
		result += name;
		logger.PrintLine(contect_str, result.c_str());
	}
	void Import(Context& context) const;
};

class ModuleAST : public HighLevelAST
{
public:
	std::shared_ptr<ModuleData> module_data_;

	std::vector<std::unique_ptr<ClassAST>> classes_;
	std::vector<std::unique_ptr<ImportAST>> imports_;
	std::vector<std::unique_ptr<StructureAST>> structures_;
	std::vector<std::unique_ptr<FunctionDeclarationAST>> functions_;

	ModuleAST() = default;
	ModuleAST(std::shared_ptr<ModuleData> module_data)
		: module_data_(module_data)
	{}

	void log(Logger& logger, const char* contect_str) const override
	{
		Utils::SoftAssert(module_data_.get(), "error..");
		std::string result = "ModuleAST ";
		result += module_data_->name;
		logger.PrintLine(contect_str, result.c_str());

		for (auto& item : classes_)
		{
			item->log(logger, "module scope class");
		}
		for (auto& item : imports_)
		{
			item->log(logger, "module scope import");
		}
		for (auto& item : structures_)
		{
			item->log(logger, "module scope structure");
		}
		for (auto& item : functions_)
		{
			item->log(logger, "module scope function");
		}
	}
	void Codegen(Context& context) const;
	void BindParsedChildren()
	{
		for (auto& func_ast : functions_)
		{
			if (func_ast && func_ast->function_data_)
			{
				func_ast->function_data_->owner = module_data_;
				module_data_->functions.emplace(func_ast->function_data_->name, func_ast->function_data_);
			}
		}

		for (auto& ast : structures_)
		{
			if (ast && ast->structure_data_)
			{
				ast->structure_data_->owner = module_data_;
				module_data_->structures.emplace(ast->structure_data_->name, ast->structure_data_);
			}
		}

		for (auto& ast : classes_)
		{
			if (ast && ast->structure_data_)
			{
				ast->structure_data_->owner = module_data_;
				module_data_->classes.emplace(ast->structure_data_->name
					, std::static_pointer_cast<ClassData>(ast->structure_data_));
			}
		}
	}
};




