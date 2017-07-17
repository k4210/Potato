#include "parser.h"
#include "lexer.h"
#include "operator_database.h"
#include "ast_expression.h"
#include "ast_flow_control.h"
#include "ast_structure.h"
#include "utils.h"
#include <assert.h> 

bool Parser::LoadExpected(Lexer& lexer, Token expected_token, const char* error_msg)
{
	if (!error_)
	{
		lexer.ReadNextToken();
		if (expected_token == lexer.GetCurrentToken().token)
		{
			return true;
		}
		error_ = true;
		LogError("Parser::ParseModule", error_msg);
	}
	return false;
}

bool Parser::ShouldReadNextToken(Lexer& lexer) const
{
	const Token token = lexer.GetCurrentToken().token;
	return !error_
		&& (Token::Error != token)
		&& (Token::EndOfFile != token);
}

std::vector<std::unique_ptr<HighLevelAST>> Parser::ParseModule(Lexer& lexer)
{
	std::vector<std::unique_ptr<HighLevelAST>> module_items;
	for(lexer.ReadNextToken(); ShouldReadNextToken(lexer); lexer.ReadNextToken())
	{
		switch (lexer.GetCurrentToken().token)
		{
			case Token::Module:
				LoadExpected(lexer, Token::Identifier, "Expected module name");
				module_items.push_back(std::make_unique<ModuleAST>(lexer.GetCurrentToken().string_value));
				LoadExpected(lexer, Token::Semicolon, "Expected ';'");
				break;
			case Token::Import:
				LoadExpected(lexer, Token::Identifier, "Expected module name");
				module_items.push_back(std::make_unique<ImportAST>(lexer.GetCurrentToken().string_value));
				LoadExpected(lexer, Token::Semicolon, "Expected ';'");
				break;
			case Token::Struct:
				module_items.push_back(ParseStruct(lexer));
				break;
			case Token::Function:
				module_items.push_back(ParseFunction(lexer));
				break;
			case Token::Class:
				module_items.push_back(ParseClass(lexer));
				break;
			default:
				error_ = true;
				LogError("Parser::ParseModule", "Unexpected token");
				break;
		}
	}
	return module_items;
}

std::unique_ptr<HighLevelAST> Parser::ParseStruct(Lexer& lexer)
{
	assert(Token::Struct == lexer.GetCurrentToken().token);
	std::unique_ptr<StructureAST> structure = std::make_unique<StructureAST>();
	if (LoadExpected(lexer, Token::Identifier, "Expected struct name"))
	{
		structure->name = lexer.GetCurrentToken().string_value;
		lexer.ReadNextToken();
		if (Token::Colon == lexer.GetCurrentToken().token)
		{
			if (LoadExpected(lexer, Token::Identifier, "Expected base struct name"))
			{
				structure->base_class = lexer.GetCurrentToken().string_value;
			}
		}
		LoadExpected(lexer, Token::OpenCurlyBracket, "Expected open scope");
		for (lexer.ReadNextToken(); ShouldReadNextToken(lexer); lexer.ReadNextToken())
		{
			if (Token::CloseCurlyBracket == lexer.GetCurrentToken().token)
			{
				lexer.ReadNextToken();
				if (Token::Semicolon == lexer.GetCurrentToken().token)
				{
					lexer.ReadNextToken();
				}
				break;
			}
			structure->member_fields.push_back(ParseMemberField(lexer));
		}
	}
	return structure;
}

VariableData Parser::ParseMemberField(Lexer& lexer)
{
	VariableData data;

	return data;
}