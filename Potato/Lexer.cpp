#include "Lexer.h"
#include "Utils.h"

#include <cstdio> //getchar
#include <ctype.h> //isalpha, isspace

#include <algorithm>

Lexer::Lexer()
	: last_read_char_(' ')
{
	alpha_token_map_.emplace("class", Token::Class);
	alpha_token_map_.emplace("interface", Token::Interface);
	alpha_token_map_.emplace("itruct", Token::Struct);
	alpha_token_map_.emplace("function", Token::Function);
	alpha_token_map_.emplace("this", Token::This);
	alpha_token_map_.emplace("object", Token::Object);
	alpha_token_map_.emplace("null", Token::Null);
	alpha_token_map_.emplace("for", Token::For);
	alpha_token_map_.emplace("continue", Token::Continue);
	alpha_token_map_.emplace("if", Token::If);
	alpha_token_map_.emplace("else", Token::Else);
	alpha_token_map_.emplace("return", Token::Return);
	alpha_token_map_.emplace("mutable", Token::Mutable);
	alpha_token_map_.emplace("mutable_ref", Token::Mutable_ref);
	alpha_token_map_.emplace("private", Token::Private);
	alpha_token_map_.emplace("public", Token::Public);
	alpha_token_map_.emplace("const", Token::Const);
	alpha_token_map_.emplace("virtual", Token::Virtual);
	alpha_token_map_.emplace("operator", Token::Operator);

	single_nonalpha_token_map_.emplace("{", Token::OpenScope);
	single_nonalpha_token_map_.emplace("}", Token::CloseScope);
	single_nonalpha_token_map_.emplace("(", Token::OpenArgumentsList);
	single_nonalpha_token_map_.emplace(")", Token::CloseArgumentsList);
	single_nonalpha_token_map_.emplace(",", Token::Separator);
	single_nonalpha_token_map_.emplace("^", Token::ReferenceSign);
	single_nonalpha_token_map_.emplace("[", Token::OpenTypeSpecifier);
	single_nonalpha_token_map_.emplace("]", Token::CloseTypeSpecifier);
	single_nonalpha_token_map_.emplace(";", Token::EndOfCommand);

	one_sign_operators.push_back('=');
	one_sign_operators.push_back('.');
	one_sign_operators.push_back('?');
	one_sign_operators.push_back(':');
	one_sign_operators.push_back('+');
	one_sign_operators.push_back('-');
	one_sign_operators.push_back('*');
	one_sign_operators.push_back('/');
	one_sign_operators.push_back('|');
	one_sign_operators.push_back('&');
	one_sign_operators.push_back('%');
	one_sign_operators.push_back('<');
	one_sign_operators.push_back('>');

	two_signs_operators.push_back("++");
	two_signs_operators.push_back("--");
	two_signs_operators.push_back("?=");
	two_signs_operators.push_back("=?");
	two_signs_operators.push_back("==");
	two_signs_operators.push_back("+=");
	two_signs_operators.push_back("-=");
	two_signs_operators.push_back("*=");
	two_signs_operators.push_back("/=");
	two_signs_operators.push_back("%=");
	two_signs_operators.push_back("&=");
	two_signs_operators.push_back("|=");
	two_signs_operators.push_back(">=");
	two_signs_operators.push_back("<=");
	//two_signs_operators.push_back("<<");
	//two_signs_operators.push_back(">>");

	//three_signs_operators.push_back("<<=");
	//three_signs_operators.push_back(">>=");
}

void Lexer::ReadNextToken()
{
	auto AppendSingleChar = [](std::string& str, int ch)
	{
		str += static_cast<char>(ch);
	};
	auto StringFromSingleSign = [](int ch) -> std::string
	{
		std::string str;
		str += static_cast<char>(ch);
		return str;
	};

	auto remove_whitepaces_and_comments = [&]()
	{
		bool ready_for_meaningful_token = false;
		do
		{
			ready_for_meaningful_token = true;
			while (isspace(last_read_char_))
			{
				last_read_char_ = getchar();
				ready_for_meaningful_token = false;
			}
			if (last_read_char_ == '#')
			{
				ready_for_meaningful_token = false;
				do // Comment until end of line.
				{
					last_read_char_ = getchar();
				} while (last_read_char_ != EOF && last_read_char_ != '\n' && last_read_char_ != '\r');
			}
		} while (!ready_for_meaningful_token);
	};
	remove_whitepaces_and_comments();

	if (last_read_char_ == EOF)
	{
		current_token_ = TokenData(Token::EndOfFile);
	}
	else if (isalpha(last_read_char_) || (last_read_char_ == '_')) // identifier: [a-zA-Z][a-zA-Z0-9]*
	{
		auto read_alphanum_token = [&]() -> TokenData
		{
			std::string idententifier = StringFromSingleSign(last_read_char_);
			do
			{
				AppendSingleChar(idententifier, last_read_char_);
				last_read_char_ = getchar();
			} while (isalnum(last_read_char_) || (last_read_char_ == '_'));
			const auto found = alpha_token_map_.find(idententifier);
			return (found != alpha_token_map_.end())
				? TokenData(found->second)
				: TokenData(Token::Identifier, idententifier);
		};
		current_token_ = read_alphanum_token();
	}
	else if (isdigit(last_read_char_))
	{
		auto read_number_token = [&]()
		{
			const int kDot = '.';
			std::string number_string;
			int dot_number = 0;
			do
			{
				if (last_read_char_ == kDot)
				{
					dot_number++;
				}
				AppendSingleChar(number_string, last_read_char_);
				last_read_char_ = getchar();
			} while (isdigit(last_read_char_) || last_read_char_ == kDot);
			if (dot_number > 1)
			{
				std::string error_msg = "Lexer error - too many dots in number: ";
				error_msg += number_string;
				LogError(error_msg);
				return TokenData(Token::Error, number_string);
			}
			const Token token = (0 == dot_number) ? Token::IntValue : Token::FloatValue;
			return TokenData(token, number_string);
		};
		current_token_ = read_number_token();
	}
	else if (last_read_char_ == '"')
	{
		//TODO:
	}
	else
	{
		const std::string idententifier = StringFromSingleSign(last_read_char_);
		auto found = single_nonalpha_token_map_.find(idententifier);
		if (found != single_nonalpha_token_map_.end())
		{
			const Token found_token = found->second;
			current_token_ = TokenData(found_token);
			last_read_char_ = getchar();
		}
		else
		{
			std::string two_signs = StringFromSingleSign(last_read_char_);
			last_read_char_ = getchar();
			AppendSingleChar(two_signs, last_read_char_);
			auto find_two_signs_operator = [&]() -> bool
			{
				auto it = std::find(two_signs_operators.begin(), two_signs_operators.end(), two_signs);
				if (it != two_signs_operators.end())
				{
					current_token_ = TokenData(Token::OperatorExpr, two_signs);
					last_read_char_ = getchar();
					return true;
				}
				return false;
			};
			bool operator_found = find_two_signs_operator();
			if (!operator_found)
			{
				auto it = std::find(one_sign_operators.begin(), one_sign_operators.end(), two_signs[0]);
				if (it != one_sign_operators.end())
				{
					std::string operator_str; operator_str = two_signs[0];
					current_token_ = TokenData(Token::OperatorExpr, operator_str);
					operator_found = true;
					//next char was already read;
				}
			}
			if (!operator_found)
			{
				std::string error_msg = "Lexer error - unknown operator: ";
				error_msg += two_signs;
				LogError(error_msg);
				current_token_ = TokenData(Token::Error, two_signs);
			}
		}
	}
}