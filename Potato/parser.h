#pragma once

#include <memory>
#include <vector>
#include "lexer.h"
#include "potato_common.h"

class HighLevelAST;
class CodeScopeAST;

class Parser
{
protected:
	bool error_ = false;
	bool eof_ = false;

	bool Expected(Lexer& lexer, Token expected_token, const char* error_msg);
	bool Expected(Lexer& lexer, Token expected_token, std::string& out_str, const char* error_msg);

	bool ShouldContinue(Lexer& lexer);

	VariableData ParseMemberField(Lexer& lexer);
	TypeData ParseType(Lexer& lexer, bool allow_void = false);
	AccessSpecifier ParseOptionalAccessSpecifier(Lexer& lexer);

	std::unique_ptr<HighLevelAST> ParseStruct(Lexer& lexer);
	std::unique_ptr<HighLevelAST> ParseFunction(Lexer& lexer);
	std::unique_ptr<HighLevelAST> ParseClass(Lexer& lexer);

	std::unique_ptr<CodeScopeAST> ParseScope(Lexer& lexer);
public:
	void Reset()
	{
		error_ = false;
		eof_ = false;
	}

	Parser()
	{
		Reset();
	}

	std::vector<std::unique_ptr<HighLevelAST>> ParseModule(Lexer& lexer);
};