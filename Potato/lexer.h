#pragma once

#include <string>
#include <map>
#include <vector>
#include <set>

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
	Void,
	True,
	False,
	Static,
	Module,
	Import,
	Enum,

	// Parantesis, brackets, etc
	OpenCurlyBracket, // {
	CloseCurlyBracket, // }
	OpenRoundBracket, // (
	CloseRoundBracket, // )
	Coma, // ,
	ReferenceSign, // ^
	OperSquareBracket, // [
	CloseSquareBracket, // ]
	Semicolon, // ;
	Dot, // .
	QuestionMark, // ? 
	Colon, // :

	DoubleColon, // ::
	Arrow, //->
	DoubleDot, // ..

	//The following tokens require a str value

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

	TokenData current_token_;
	int last_read_char_;

	//maps of context-free tokens
	std::map<std::string, Token> alpha_token_map_;
	std::map<int, Token> single_nonalpha_token_map_;
	std::map<std::string, Token> two_signs_nonalpha_token_map_;

	std::set<int> one_sign_operators_;
	std::set<std::string> two_signs_operators_;

	void ReadNextToken();
public:

	void Start() 
	{ 
		ReadNextToken(); 
	}
	void End() {}
	// if given token is equal current one, then next token is read, and true is return
	bool Consume(Token token);
	bool Consume(Token token, std::string& out_str);
	void RegisterOperatorString(const char* str);
	Lexer();
};
