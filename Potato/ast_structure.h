#pragma once

#include "ast_expression.h"
#include "ast_flow_control.h"

class HighLevelAST
{
public:
	virtual ~HighLevelAST() = default;
	virtual void log(Logger& logger, const char* contect_str) const = 0;
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
		if (optional_body)
		{
			optional_body->log(logger, "body");
		}
		logger.DecreaseIndent();
	}
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
	}
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

		for (auto& func : functions)
		{
			func->log(logger, "func");
		}
	}
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
};



