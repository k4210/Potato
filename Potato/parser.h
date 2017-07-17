#pragma once

#include <memory>
#include <vector>
#include "lexer.h"
#include "potato_common.h"

class HighLevelAST;
class Lexer;

class Parser
{
	bool error_;
public:
	bool LoadExpected(Lexer& lexer, Token expected_token, const char* error_msg);
	bool ShouldReadNextToken(Lexer& lexer) const;

	std::unique_ptr<HighLevelAST> ParseStruct(Lexer& lexer);
	std::unique_ptr<HighLevelAST> ParseFunction(Lexer& lexer);
	std::unique_ptr<HighLevelAST> ParseClass(Lexer& lexer);
	VariableData ParseMemberField(Lexer& lexer);

	std::vector<std::unique_ptr<HighLevelAST>> ParseModule(Lexer& lexer);

};