#include "parser.h"
#include "lexer.h"
#include "operator_database.h"
#include "ast_expression.h"
#include "ast_flow_control.h"
#include "ast_structure.h"
#include "utils.h"

std::vector<std::unique_ptr<HighLevelAST>> Parser::ParseModule(Lexer& lexer)
{
	std::vector<std::unique_ptr<HighLevelAST>> module_items;
	bool error = false;
	auto IsValidToken = [](Token token) -> bool
	{
		return Token::Error != token && Token::EndOfFile != token;
	};
	auto LoadExpected = [&lexer, &error](Token expected_token, const char* error_msg) -> bool
	{
		if (!error)
		{
			lexer.ReadNextToken();
			if (expected_token == lexer.GetCurrentToken().token)
			{
				return true;
			}
			error = true;
			LogError("Parser::ParseModule", error_msg);
		}
		return false;
	};
	for(lexer.ReadNextToken(); IsValidToken(lexer.GetCurrentToken().token) && !error; lexer.ReadNextToken())
	{
		switch (lexer.GetCurrentToken().token)
		{
			case Token::Module:
				LoadExpected(Token::Identifier, "Expected module name");
				module_items.push_back(std::make_unique<ModuleAST>(lexer.GetCurrentToken().string_value));
				LoadExpected(Token::Semicolon, "Expected ';'");
				break;
			case Token::Import:
				LoadExpected(Token::Identifier, "Expected module name");
				module_items.push_back(std::make_unique<ImportAST>(lexer.GetCurrentToken().string_value));
				LoadExpected(Token::Semicolon, "Expected ';'");
				break;
			case Token::Struct:
			{

			}
			break;
			case Token::Function:
			{

			}
			break;
			case Token::Class:
			{

			}
			break;
		}
	}
	if (Token::Error == lexer.GetCurrentToken().token)
	{
		error = true;
		LogError("Parser::ParseModule", "Invalid token");
	}
	return module_items;
}