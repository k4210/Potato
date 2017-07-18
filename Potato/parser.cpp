#include "parser.h"
#include "lexer.h"
#include "operator_database.h"
#include "ast_expression.h"
#include "ast_flow_control.h"
#include "ast_structure.h"
#include "utils.h"

bool Parser::Expected(Lexer& lexer, Token expected_token, const char* error_msg)
{
	if (!error_)
	{
		if (lexer.Consume(expected_token))
		{
			return true;
		}
		error_ = true;
		LogError("Parser::ParseModule", error_msg);
	}
	return false;
}

bool Parser::Expected(Lexer& lexer, Token expected_token, std::string& out_str, const char* error_msg)
{
	if (!error_)
	{
		if (lexer.Consume(expected_token, out_str))
		{
			return true;
		}
		error_ = true;
		LogError("Parser::ParseModule", error_msg);
	}
	return false;
}

bool Parser::ShouldContinue(Lexer& lexer)
{
	error_ |= lexer.Consume(Token::Error);
	eof_ |= lexer.Consume(Token::EndOfFile);

	return !error_ && !eof_;
}

TypeData Parser::ParseType(Lexer& lexer, bool allow_void)
{
	TypeData type_data;
	while (ShouldContinue(lexer))
	{
		if (lexer.Consume(Token::Mutable))
		{
			type_data.mutable_specifiers |= static_cast<unsigned int>(MutableSpecifier::Mutable);
		}
		else if (lexer.Consume(Token::Mutable_ref))
		{
			type_data.mutable_specifiers |= static_cast<unsigned int>(MutableSpecifier::MutableRef);
		}
		else
		{
			break;
		}
	}

	if (allow_void && lexer.Consume(Token::Void))
	{
		type_data.type = EType::Void;
		return type_data;
	}

	Expected(lexer, Token::Identifier, type_data.name, "Expected a type");
	if (lexer.Consume(Token::ReferenceSign))
	{
		type_data.type = EType::Reference;
	}
	return type_data;
}

AccessSpecifier Parser::ParseOptionalAccessSpecifier(Lexer& lexer)
{
	if (lexer.Consume(Token::Public))
	{
		return AccessSpecifier::Public;
	}
	else if (lexer.Consume(Token::Private))
	{
		return AccessSpecifier::Private;
	}
	return AccessSpecifier::Default;
}

VariableData Parser::ParseMemberField(Lexer& lexer)
{
	VariableData data;
	data.access_specifier = ParseOptionalAccessSpecifier(lexer);
	data.type_data = ParseType(lexer);

	Expected(lexer, Token::Identifier, data.name, "Expected a variable name");
	Expected(lexer, Token::Semicolon, "expected ;");

	return data;
}

std::unique_ptr<HighLevelAST> Parser::ParseStruct(Lexer& lexer)
{
	std::unique_ptr<StructureAST> structure = std::make_unique<StructureAST>();
	if (Expected(lexer, Token::Identifier, structure->name, "Expected struct name"))
	{
		if (lexer.Consume(Token::Colon))
		{
			Expected(lexer, Token::Identifier, structure->base_class, "Expected base struct name");
		}
		Expected(lexer, Token::OpenCurlyBracket, "Expected open scope");
		while(ShouldContinue(lexer))
		{
			if (lexer.Consume(Token::CloseCurlyBracket))
			{
				lexer.Consume(Token::Semicolon);
				break;
			}
			structure->member_fields.push_back(ParseMemberField(lexer));
		}
	}
	return structure;
}

std::unique_ptr<HighLevelAST> Parser::ParseFunction(Lexer& lexer)
{
	std::unique_ptr<FunctionDeclarationAST> function = std::make_unique<FunctionDeclarationAST>();
	function->acces_specifier = ParseOptionalAccessSpecifier(lexer);
	function->return_type = ParseType(lexer, true);
	Expected(lexer, Token::Identifier, function->name, "Expected name");
	Expected(lexer, Token::OpenRoundBracket, "Expected (");
	if (!lexer.Consume(Token::CloseRoundBracket))
	{
		while (ShouldContinue(lexer))
		{
			VariableData parameter;
			parameter.type_data = ParseType(lexer);
			Expected(lexer, Token::Identifier, parameter.name, "Expected parameter name");
			function->parameters.push_back(parameter);
			if (lexer.Consume(Token::CloseRoundBracket))
			{
				break;
			}
			Expected(lexer, Token::Coma, "Expecter ')' or ','");
		}
	}
	function->is_mutable = lexer.Consume(Token::Mutable);
	if (!lexer.Consume(Token::Semicolon))
	{
		function->optional_body = ParseScope(lexer);
	}
	return function;
}

std::unique_ptr<HighLevelAST> Parser::ParseClass(Lexer& lexer)
{
	std::unique_ptr<ClassAST> class_ast = std::make_unique<ClassAST>();
	//TODO:
	return class_ast;
}

std::vector<std::unique_ptr<HighLevelAST>> Parser::ParseModule(Lexer& lexer)
{
	std::vector<std::unique_ptr<HighLevelAST>> module_items;
	while (ShouldContinue(lexer))
	{
		std::string identifier;
		if (lexer.Consume(Token::Module))
		{
			Expected(lexer, Token::Identifier, identifier, "Expected module name");
			module_items.push_back(std::make_unique<ModuleAST>(identifier));
			Expected(lexer, Token::Semicolon, "Expected ';'");
		}
		else if (lexer.Consume(Token::Import))
		{
			Expected(lexer, Token::Identifier, identifier, "Expected module name");
			module_items.push_back(std::make_unique<ImportAST>(identifier));
			Expected(lexer, Token::Semicolon, "Expected ';'");
		}
		else if (lexer.Consume(Token::Struct))
		{
			module_items.push_back(ParseStruct(lexer));
		}
		else if (lexer.Consume(Token::Function))
		{
			module_items.push_back(ParseFunction(lexer));
		}
		else if (lexer.Consume(Token::Class))
		{
			module_items.push_back(ParseClass(lexer));
		}
		else
		{
			error_ = true;
			LogError("Parser::ParseModule", "Unexpected token");
		}
	}
	return module_items;
}

std::unique_ptr<CodeScopeAST> Parser::ParseScope(Lexer& lexer)
{
	Expected(lexer, Token::OpenCurlyBracket, "Expected {");
	std::vector<std::unique_ptr<NodeAST>> code;

	//TODO:

	return std::make_unique<CodeScopeAST>(code);
}