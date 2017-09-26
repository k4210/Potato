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
	void RegisterFunction(Context& context) const;
	void Codegen(Context& context) const;
};

class StructureAST : public HighLevelAST
{
public:
	//Todo: add info - expose to other modules? yes
	std::shared_ptr<StructData> structure_data_;

	StructureAST() = default;
	StructureAST(std::shared_ptr<StructData> structure_data)
		: structure_data_(structure_data)
	{}

	void log(Logger& logger, const char* contect_str) const override
	{
		Utils::SoftAssert(structure_data_.get(), "error..");
		std::string result = "StructureAST ";
		result += structure_data_->name;
		logger.PrintLine(contect_str, result.c_str());

		logger.IncreaseIndent();
		for (auto& var : structure_data_->member_fields)
		{
			logger.PrintLine("member_field", var.ToString().c_str());
		}
		logger.DecreaseIndent();
	}
	void RegisterType(Context& context) const;
	void GenerateDataLayout(Context& context) const;
	void RegisterFunctions(Context& context) const;
	void Codegen(Context& context) const;
};

class ClassAST : public HighLevelAST
{
public:
	std::shared_ptr<ClassData> class_data_;

	std::vector<std::unique_ptr<FunctionDeclarationAST>> functions_;

	ClassAST() = default;
	ClassAST(std::shared_ptr<ClassData> class_data)
		: class_data_(class_data)
	{}

	void log(Logger& logger, const char* contect_str) const override
	{
		Utils::SoftAssert(class_data_.get(), "error..");
		std::string result = "ClassAST ";
		result += class_data_->name;
		result += " : ";
		result += class_data_->base_class;
		logger.PrintLine(contect_str, result.c_str());
		logger.IncreaseIndent();
		for (auto& var : class_data_->member_fields)
		{
			logger.PrintLine("member_field", var.ToString().c_str());
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
				func_ast->function_data_->owner = class_data_;
				class_data_->functions.emplace_back(func_ast->function_data_);
			}
		}
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
				module_data_->functions.emplace_back(func_ast->function_data_);
			}
		}

		for (auto& ast : structures_)
		{
			if (ast && ast->structure_data_)
			{
				ast->structure_data_->owner = module_data_;
				module_data_->structures.emplace_back(ast->structure_data_);
			}
		}

		for (auto& ast : classes_)
		{
			if (ast && ast->class_data_)
			{
				ast->class_data_->owner = module_data_;
				module_data_->classes.emplace_back(ast->class_data_);
			}
		}
	}
};




