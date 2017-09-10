#include "stdafx.h"
#include "lexer.h"
#include "utils.h"

Lexer::Lexer()
	: last_read_char_(' ')
{
	alpha_token_map_.emplace("class", EToken::Class);
	alpha_token_map_.emplace("interface", EToken::Interface);
	alpha_token_map_.emplace("struct", EToken::Struct);
	alpha_token_map_.emplace("function", EToken::Function);
	alpha_token_map_.emplace("this", EToken::This);
	alpha_token_map_.emplace("object", EToken::Object);
	alpha_token_map_.emplace("null", EToken::Null);
	alpha_token_map_.emplace("for", EToken::For);
	alpha_token_map_.emplace("break", EToken::Break);
	alpha_token_map_.emplace("continue", EToken::Continue);
	alpha_token_map_.emplace("if", EToken::If);
	alpha_token_map_.emplace("else", EToken::Else);
	alpha_token_map_.emplace("return", EToken::Return);
	alpha_token_map_.emplace("mutable", EToken::Mutable);
	alpha_token_map_.emplace("mutable_ref", EToken::Mutable_ref);
	alpha_token_map_.emplace("private", EToken::Private);
	alpha_token_map_.emplace("public", EToken::Public);
	alpha_token_map_.emplace("const", EToken::Const);
	alpha_token_map_.emplace("virtual", EToken::Virtual);
	alpha_token_map_.emplace("operator", EToken::Operator);
	alpha_token_map_.emplace("void", EToken::Void);
	alpha_token_map_.emplace("true", EToken::True);
	alpha_token_map_.emplace("false", EToken::False);
	alpha_token_map_.emplace("static", EToken::Static);
	alpha_token_map_.emplace("module", EToken::Module);
	alpha_token_map_.emplace("import", EToken::Import);
	alpha_token_map_.emplace("enum", EToken::Enum);
	alpha_token_map_.emplace("new", EToken::New);
	alpha_token_map_.emplace("int", EToken::Int);
	alpha_token_map_.emplace("flpat", EToken::Float);

	two_signs_nonalpha_token_map_.emplace("..", EToken::DoubleDot);
	two_signs_nonalpha_token_map_.emplace("->", EToken::Arrow);
	two_signs_nonalpha_token_map_.emplace("::", EToken::DoubleColon);

	single_nonalpha_token_map_.emplace('{', EToken::OpenCurlyBracket);
	single_nonalpha_token_map_.emplace('}', EToken::CloseCurlyBracket);
	single_nonalpha_token_map_.emplace('(', EToken::OpenRoundBracket);
	single_nonalpha_token_map_.emplace(')', EToken::CloseRoundBracket);
	single_nonalpha_token_map_.emplace(',', EToken::Coma);
	single_nonalpha_token_map_.emplace('^', EToken::ReferenceSign);
	single_nonalpha_token_map_.emplace('[', EToken::OpenSquareBracket);
	single_nonalpha_token_map_.emplace(']', EToken::CloseSquareBracket);
	single_nonalpha_token_map_.emplace(';', EToken::Semicolon);
	single_nonalpha_token_map_.emplace('.', EToken::Dot);
	single_nonalpha_token_map_.emplace('?', EToken::QuestionMark);
	single_nonalpha_token_map_.emplace(':', EToken::Colon);
}

void Lexer::RegisterOperatorString(const char* str)
{
	const size_t str_size = strlen(str);
	if (1 == str_size)
	{
		one_sign_operators_.emplace(*str);
	}
	else if (2 == str_size)
	{
		two_signs_operators_.emplace(str);
	}
	else
	{
		Utils::LogError(GetCodeLocation().c_str(), "Lexer::RegisterOperatorString wrong size of: ", str);
	}
}
namespace 
{
	static void AppendSingleChar(std::string& str, int ch)
	{
		str += static_cast<char>(ch);
	}

	static std::string StringFromSingleSign(int ch)
	{
		std::string str;
		AppendSingleChar(str, ch);
		return str;
	}
}

bool Lexer::Consume(EToken token)
{
	if (current_token_.token == token)
	{
		ReadNextToken();
		return true;
	}
	return false;
}

bool Lexer::Consume(EToken token, std::string& out_str)
{
	if (current_token_.token == token)
	{
		out_str = std::move(current_token_.string_value);
		ReadNextToken();
		return true;
	}
	return false;
}

void Lexer::ReadNextToken()
{
	if (EToken::EndOfFile == current_token_.token || EToken::Error == current_token_.token)
	{
		return;
	}

	auto getchar = [&]() -> int
	{
		const int read_char = input_stream_.get();
		if (read_char == '\n')
		{
			current_line_++;
			sign_in_line_ = 0;
		}
		else
		{
			sign_in_line_++;
		}
		return read_char;
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
		current_token_ = TokenData(EToken::EndOfFile);
	}
	else if (isalpha(last_read_char_) || (last_read_char_ == '_'))
	{
		auto read_alphanum_token = [&]() -> TokenData
		{
			std::string idententifier;
			do
			{
				AppendSingleChar(idententifier, last_read_char_);
				last_read_char_ = getchar();
			} while (isalnum(last_read_char_) || (last_read_char_ == '_'));
			const auto found = alpha_token_map_.find(idententifier);
			return (found != alpha_token_map_.end())
				? TokenData(found->second)
				: TokenData(EToken::Identifier, idententifier);
		};
		current_token_ = read_alphanum_token();
	}
	else if (isdigit(last_read_char_))
	{
		auto read_number_token = [&]() -> TokenData
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
				Utils::LogError(GetCodeLocation().c_str(),"Lexer error - too many dots in number: ", number_string.c_str());
				return TokenData(EToken::Error, number_string);
			}
			const EToken token = (0 == dot_number) ? EToken::IntValue : EToken::FloatValue;
			return TokenData(token, number_string);
		};
		current_token_ = read_number_token();
	}
	else if (last_read_char_ == '"')
	{
		auto read_string_token = [&]() -> TokenData
		{
			std::string str_value;
			for (last_read_char_ = getchar()
				; last_read_char_ != '"' && last_read_char_ != EOF
				; last_read_char_ = getchar())
			{
				AppendSingleChar(str_value, last_read_char_);
			}
			if (last_read_char_ != EOF)
			{
				last_read_char_ = getchar();
				return TokenData(EToken::StringValue, str_value);
			}
			Utils::LogError("Lexer error - never ending string");
			return TokenData(EToken::Error, str_value);
		};
		current_token_ = read_string_token();
	}
	else
	{
		auto read_nonalphanum_stuff = [&]() -> TokenData
		{
			std::string two_signs = StringFromSingleSign(last_read_char_);
			last_read_char_ = getchar();
			AppendSingleChar(two_signs, last_read_char_);

			auto two_sign_token_found = two_signs_nonalpha_token_map_.find(two_signs);
			if (two_sign_token_found != two_signs_nonalpha_token_map_.end())
			{
				last_read_char_ = getchar();
				return TokenData(two_sign_token_found->second);
			}

			const bool two_sign_op_found = two_signs_operators_.find(two_signs) != two_signs_operators_.end();
			if (two_sign_op_found)
			{
				last_read_char_ = getchar();
				return TokenData(EToken::OperatorExpr, two_signs);
			}

			auto found_single_nonalpha_token = single_nonalpha_token_map_.find(two_signs[0]);
			if (found_single_nonalpha_token != single_nonalpha_token_map_.end())
			{
				//next char was already read;
				return TokenData(found_single_nonalpha_token->second);
			}

			const bool single_sign_op_found = one_sign_operators_.find(two_signs[0]) != one_sign_operators_.end();
			if (single_sign_op_found)
			{
				//next char was already read;
				return TokenData(EToken::OperatorExpr, StringFromSingleSign(two_signs[0]));
			}
			Utils::LogError(GetCodeLocation().c_str(), "Lexer error - unknown operator: ", two_signs.c_str());
			return TokenData(EToken::Error, two_signs);
		};
		current_token_ = read_nonalphanum_stuff();
	}
}

std::string Lexer::GetCodeLocation() const
{
	std::string result = filename_ + " ";
	result += std::to_string(current_line_) + " ";
	result += std::to_string(sign_in_line_);
	return result;
}

void Lexer::Start(const char* filename)
{
	input_stream_.open(filename);
	current_line_ = 1;
	sign_in_line_ = 0;
	filename_ = std::string(filename);

	if (!input_stream_.is_open())
	{
		current_token_.token = EToken::Error;
		Utils::LogError("Lexer::Start failed to open file", filename);
		return;
	}
	current_token_.token = EToken::Void;
	last_read_char_ = ' ';
	ReadNextToken();
}
void Lexer::End()
{
	input_stream_.close();
}