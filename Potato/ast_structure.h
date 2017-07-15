#pragma once

#include "ast_expression.h"
#include "ast_flow_control.h"

class HighLevelAST
{
public:
	virtual ~HighLevelAST() = default;
	virtual llvm::Value *codegen() = 0;
};

class FunctionDeclarationAST : public HighLevelAST
{
	AccessSpecifier acces_specifier;
	TypeData return_type;
	std::vector<VariableData> parameters;

	std::unique_ptr<CodeScopeAST> optional_body;

	llvm::Value *codegen() override { return nullptr; }
};

class ClassAST : public HighLevelAST
{
	AccessSpecifier acces_specifier;
	std::string name;
	std::string base_class;
	std::vector<std::string> implemented_interfaces;
	std::vector<VariableData> member_fields;
	std::vector<std::unique_ptr<FunctionDeclarationAST>> functions;

	llvm::Value *codegen() override { return nullptr; }
};

class StructureAST : public HighLevelAST
{
	AccessSpecifier acces_specifier;
	std::string name;
	std::string base_class;
	std::vector<VariableData> member_fields;

	llvm::Value *codegen() override { return nullptr; }
};

