#pragma once

#include "potato_common.h"

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

	std::ifstream input_stream_;
	int current_line_ = 0;
	int sign_in_line_ = 0;
	std::string filename_;

	void ReadNextToken();
public:

	void Start(const char* filename);
	void End();

	std::string GetCodeLocation() const;

	bool Consume(Token token);
	bool Consume(Token token, std::string& out_str);
	bool CheckTokenDontProceed(Token token) const
	{
		return (current_token_.token == token);
	}
	void RegisterOperatorString(const char* str);
	Lexer();
	~Lexer()
	{
		End();
	}
};
