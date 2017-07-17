#pragma once

#include "ast_expression.h"
#include "ast_flow_control.h"

class HighLevelAST
{
public:
	virtual ~HighLevelAST() = default;
};

class FunctionDeclarationAST : public HighLevelAST
{
	AccessSpecifier acces_specifier;
	TypeData return_type;
	std::vector<VariableData> parameters;

	std::unique_ptr<CodeScopeAST> optional_body;
};

class StructureAST : public HighLevelAST
{
public:
	//Todo: add info - expose to other modules?
	std::string name;
	std::string base_class;
	std::vector<VariableData> member_fields;
};

class ClassAST : public StructureAST
{
	std::vector<std::string> implemented_interfaces;
	std::vector<std::unique_ptr<FunctionDeclarationAST>> functions;
};

class ModuleAST : public HighLevelAST
{
public:
	std::string name;

	ModuleAST(const std::string& in_name)
		: name(in_name)
	{}
};

class ImportAST : public HighLevelAST
{
public:
	std::string name;

	ImportAST(const std::string& in_name)
		: name(in_name)
	{}
};



