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
	std::string name;
	AccessSpecifier acces_specifier;
	TypeData return_type;
	std::vector<VariableData> parameters;
	bool is_mutable = false;

	std::unique_ptr<CodeScopeAST> optional_body;

	void log(Logger& logger, const char* contect_str) const override
	{
		std::string result = "FunctionDeclarationAST ";
		result += name;
		logger.PrintLine(contect_str, result.c_str());
		logger.IncreaseIndent();
		logger.PrintLine("return", return_type.ToString().c_str());
		for (auto& param : parameters)
		{
			logger.PrintLine("parameter", param.ToString().c_str());
		}
		if (optional_body)
		{
			optional_body->log(logger, "body");
		}
		logger.DecreaseIndent();
	}
	void codegen(Context& context) const override;
};

class StructureAST : public HighLevelAST
{
public:
	//Todo: add info - expose to other modules?
	std::string name;
	std::string base_class;
	std::vector<VariableData> member_fields;

	void log(Logger& logger, const char* contect_str) const override
	{
		std::string result = "StructureAST ";
		result += name;
		logger.PrintLine(contect_str, result.c_str());

		logger.IncreaseIndent();
		for (auto& var : member_fields)
		{
			logger.PrintLine("member_field", var.ToString().c_str());
		}
		logger.DecreaseIndent();
	}
	void codegen(Context& context) const override;
};

class ClassAST : public StructureAST
{
public:
	std::vector<std::string> implemented_interfaces;
	std::vector<std::unique_ptr<FunctionDeclarationAST>> functions;

	void log(Logger& logger, const char* contect_str) const override
	{
		std::string result = "ClassAST ";
		result += name;
		logger.PrintLine(contect_str, result.c_str());
		logger.IncreaseIndent();
		for (auto& var : member_fields)
		{
			logger.PrintLine("member_field", var.ToString().c_str());
		}

		for (auto& func : functions)
		{
			func->log(logger, "func");
		}
		logger.DecreaseIndent();
	}
	void codegen(Context& context) const override;
};

class ModuleAST : public HighLevelAST
{
public:
	std::string name;
	std::vector<std::unique_ptr<HighLevelAST>> items;

	void log(Logger& logger, const char* contect_str) const override
	{
		std::string result = "ModuleAST ";
		result += name;
		logger.PrintLine(contect_str, result.c_str());

		for (auto& item : items)
		{
			item->log(logger, "global");
		}
	}
	void codegen(Context& context) const override;
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
	void codegen(Context& context) const override;
};



