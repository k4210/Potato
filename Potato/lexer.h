#pragma once

#include "potato_common.h"

class Lexer
{
	struct TokenData
	{
		EToken token;
		std::string string_value;

		TokenData() 
			: token(EToken::Error)
		{}

		TokenData(EToken in_token) 
			: token(in_token)
		{}

		TokenData(EToken in_token, const std::string& in_string) 
			: token(in_token)
			, string_value(in_string) 
		{}
	};

	TokenData current_token_;
	int last_read_char_;

	//maps of context-free tokens
	std::map<std::string, EToken> alpha_token_map_;
	std::map<int, EToken> single_nonalpha_token_map_;
	std::map<std::string, EToken> two_signs_nonalpha_token_map_;

	std::set<int> one_sign_operators_;
	std::set<std::string> two_signs_operators_;

	std::ifstream input_stream_;

	// THe data are needed to "get code location as string"
	int current_line_ = 0;
	int sign_in_line_ = 0;
	std::string filename_;

	void ReadNextToken();
public:

	void Start(const char* filename);
	void End();

	std::string GetCodeLocation() const;

	bool Consume(EToken token);
	bool Consume(EToken token, std::string& out_str);
	bool CheckTokenDontProceed(EToken token) const
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
