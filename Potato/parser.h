#pragma once

#include "potato_common.h"

class Lexer;
class StructureAST;
class FunctionDeclarationAST;
class ClassAST;
class CodeScopeAST;
class ExprAST;
class ModuleAST;

class Parser
{
protected:
	bool error_ = false;
	bool eof_ = false;
	Lexer& lexer_;

public:
	bool Expected(Token expected_token, const char* error_msg = nullptr);
	bool Expected(Token expected_token, std::string& out_str, const char* error_msg = nullptr);
	bool Optional(Token token);
	bool Optional(Token token, std::string& out_str);
	bool ShouldContinue();

	VariableData ParseMemberField();
	TypeData ParseType(bool allow_void = false);
	AccessSpecifier ParseOptionalAccessSpecifier();
	void LogError(const char* msg1, const char* msg2 = nullptr);

	std::unique_ptr<StructureAST> ParseStruct();
	std::unique_ptr<FunctionDeclarationAST> ParseFunction();
	std::unique_ptr<ClassAST> ParseClass();

	std::unique_ptr<CodeScopeAST> ParseScope();
	std::unique_ptr<NodeAST> ParseFlowControl();
	std::unique_ptr<ExprAST> ParseExpression(const std::string* already_read_id = nullptr);
	std::unique_ptr<ExprAST> ParseWholeExpressionLine();

public:
	Parser(Lexer& lexer) 
		: lexer_(lexer)
	{}

	std::unique_ptr<ModuleAST> ParseModule();
};