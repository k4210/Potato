#pragma once

#include <string>
#include <map>
#include <vector>

/*
Keywords:
- class, interface, struct, function
- this, null
- for, continue, break
- if, else
- return
- mutable, mutable_ref
- private, public
- const
- virtual
- operator

Built-in types:
- int32, float32, uint32
- string, vector<>
- object-strong-ref, object-weak-ref


*/

enum class Token
{
	EndOfFile,

	//keywords
	Class, 
	Interface, 
	Struct, 
	Function,
	This, 
	Object,
	Null,
	For,
	Continue,
	Break,
	If,
	Else,
	Return,
	Mutable, 
	Mutable_ref,
	Private,
	Public,
	Const,
	Virtual, 
	Operator,

	// Parantesis, brackets, etc
	OpenScope, // {
	CloseScope, // }
	OpenArgumentsList, // (
	CloseArgumentsList, // )
	Separator, // ,
	ReferenceSign, // ^
	OpenTypeSpecifier, // [
	CloseTypeSpecifier, // ]
	EndOfCommand, // ;

	//literals
	IntValue,
	FloatValue,
	StringValue,

	//other
	OperatorExpr,
	Identifier,
	Error,
};

class Lexer
{
public:
	struct TokenData
	{
		Token token;
		std::string string_value;

		TokenData() 
			: token(Token::Error)
		{}

		TokenData(Token in_token) 
			: token(in_token)
		{}

		TokenData(Token in_token, const std::string& in_string) 
			: token(in_token)
			, string_value(in_string) 
		{}
	};

private:
	TokenData current_token_;
	int last_read_char_;

	//maps of context-free tokens
	std::map<std::string, Token> alpha_token_map_;
	std::map<std::string, Token> single_nonalpha_token_map_;

	std::vector<int> one_sign_operators;
	std::vector<std::string> two_signs_operators;
	//std::vector<std::string> three_signs_operators;

public:
	const TokenData& GetCurrentToken() const
	{
		return current_token_;
	}

	void ReadNextToken();

	Lexer();
};