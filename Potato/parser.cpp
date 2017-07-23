#include "stdafx.h"
#include "parser.h"
#include "operator_database.h"
#include "lexer.h"
#include "ast_expression.h"
#include "ast_flow_control.h"
#include "ast_structure.h"
#include "utils.h"

void Parser::LogError(const char* msg1, const char* msg2)
{
	error_ = true;
	Utils::LogError(lexer_.GetCodeLocation(), msg1, msg2);
}

bool Parser::Optional(Token token)
{
	return lexer_.Consume(token);
}
bool Parser::Optional(Token token, std::string& out_str)
{
	return lexer_.Consume(token, out_str);
}

bool Parser::Expected(Token expected_token, const char* error_msg)
{
	if (!error_)
	{
		if (lexer_.Consume(expected_token))
		{
			return true;
		}
		if (nullptr == error_msg)
		{
			// alloca generic msg
		}
		LogError(error_msg);
	}
	return false;
}

bool Parser::Expected(Token expected_token, std::string& out_str, const char* error_msg)
{
	if (!error_)
	{
		if (lexer_.Consume(expected_token, out_str))
		{
			return true;
		}
		if (nullptr == error_msg)
		{
			// alloca generic msg
		}
		LogError(error_msg);
	}
	return false;
}

bool Parser::ShouldContinue()
{
	error_ |= lexer_.Consume(Token::Error);
	eof_ |= lexer_.Consume(Token::EndOfFile);

	return !error_ && !eof_;
}

struct TypeParserHelper
{
	enum class TypeParseResult
	{
		Unknown,
		Type,
		JustId,
		//To immediate return
		NotAType,
		Error,
	};

	static TypeParseResult OptionalParseType(Parser& parser, bool allow_void, TypeData& out_type)
	{
		if (allow_void && parser.Optional(Token::Void))
		{
			out_type.type = EType::Void;
			return TypeParseResult::Type;
		}

		TypeParseResult result = TypeParseResult::Unknown;
		while (parser.ShouldContinue())
		{
			if (parser.Optional(Token::Mutable))
			{
				out_type.mutable_specifiers |= static_cast<unsigned int>(MutableSpecifier::Mutable);
				result = TypeParseResult::Type;
			}
			else if (parser.Optional(Token::Mutable_ref))
			{
				out_type.mutable_specifiers |= static_cast<unsigned int>(MutableSpecifier::MutableRef);
				result = TypeParseResult::Type;
			}
			else
			{
				break;
			}
		}

		if (!parser.Optional(Token::Identifier, out_type.name))
		{
			if (TypeParseResult::Type == result)
			{
				return TypeParseResult::Error;
			}
			return TypeParseResult::NotAType;
		}
		else if (TypeParseResult::Unknown == result)
		{
			result = TypeParseResult::JustId;
		}
		if (parser.Optional(Token::ReferenceSign))
		{
			out_type.type = EType::Reference;
			result = TypeParseResult::Type;
		}
		return result;
	}
};

TypeData Parser::ParseType(bool allow_void)
{
	TypeData type_data;
	const TypeParserHelper::TypeParseResult result = TypeParserHelper::OptionalParseType(*this, allow_void, type_data);
	const bool valid_type = (result == TypeParserHelper::TypeParseResult::Type) || (result == TypeParserHelper::TypeParseResult::JustId);
	if (!valid_type)
	{
		LogError("Parser::ParseType", "Expected a type");
	}
	return type_data;
}

AccessSpecifier Parser::ParseOptionalAccessSpecifier()
{
	if (Optional(Token::Public))
	{
		return AccessSpecifier::Public;
	}
	else if (Optional(Token::Private))
	{
		return AccessSpecifier::Private;
	}
	return AccessSpecifier::Default;
}

VariableData Parser::ParseMemberField()
{
	VariableData data;
	data.access_specifier = ParseOptionalAccessSpecifier();
	data.type_data = ParseType();

	Expected(Token::Identifier, data.name, "Expected a variable name");
	Expected(Token::Semicolon, "expected ;");

	return data;
}

std::unique_ptr<StructureAST> Parser::ParseStruct()
{
	std::unique_ptr<StructureAST> structure = std::make_unique<StructureAST>();
	Expected(Token::Identifier, structure->name, "Expected struct name");
	if (Optional(Token::Colon))
	{
		Expected(Token::Identifier, structure->base_class, "Expected base struct name");
	}
	Expected(Token::OpenCurlyBracket, "Expected open scope");
	while(ShouldContinue())
	{
		if (Optional(Token::CloseCurlyBracket))
		{
			Optional(Token::Semicolon);
			break;
		}
		structure->member_fields.push_back(ParseMemberField());
	}
	return structure;
}

std::unique_ptr<FunctionDeclarationAST> Parser::ParseFunction()
{
	std::unique_ptr<FunctionDeclarationAST> function = std::make_unique<FunctionDeclarationAST>();
	function->acces_specifier = ParseOptionalAccessSpecifier();
	function->return_type = ParseType(true);
	Expected(Token::Identifier, function->name, "Expected name");
	Expected(Token::OpenRoundBracket, "Expected (");
	if (!Optional(Token::CloseRoundBracket))
	{
		while (ShouldContinue())
		{
			VariableData parameter;
			parameter.type_data = ParseType();
			Expected(Token::Identifier, parameter.name, "Expected parameter name");
			function->parameters.push_back(parameter);
			if (Optional(Token::CloseRoundBracket))
			{
				break;
			}
			Expected(Token::Coma, "Expecter ')' or ','");
		}
	}
	function->is_mutable = Optional(Token::Mutable);
	if (!Optional(Token::Semicolon))
	{
		Expected(Token::OpenCurlyBracket, "Expected {");
		function->optional_body = ParseScope();
	}
	return function;
}

std::unique_ptr<ClassAST> Parser::ParseClass()
{
	std::unique_ptr<ClassAST> class_ast = std::make_unique<ClassAST>();
	Expected(Token::Identifier, class_ast->name, "Expected struct name");
	if (Optional(Token::Colon))
	{
		Expected(Token::Identifier, class_ast->base_class, "Expected base struct name");
	}
	Expected(Token::OpenCurlyBracket, "Expected open scope");
	while (ShouldContinue())
	{
		if (Optional(Token::CloseCurlyBracket))
		{
			Optional(Token::Semicolon);
			break;
		}
		if (Optional(Token::Function))
		{
			class_ast->functions.push_back(ParseFunction());
		}
		else
		{
			class_ast->member_fields.push_back(ParseMemberField());
		}
	}
	return class_ast;
}

std::unique_ptr<ModuleAST> Parser::ParseModule()
{
	std::unique_ptr<ModuleAST> module = std::make_unique<ModuleAST>();
	if (Optional(Token::Module))
	{
		Expected(Token::Identifier, module->name, "Expected module name");
		Expected(Token::Semicolon, "Expected ';'");
	}
	while (ShouldContinue())
	{
		std::string identifier;
		if (Optional(Token::Import))
		{
			Expected(Token::Identifier, identifier, "Expected module name");
			module->items.push_back(std::make_unique<ImportAST>(identifier));
			Expected(Token::Semicolon, "Expected ';'");
		}
		else if (Optional(Token::Struct))
		{
			module->items.push_back(ParseStruct());
		}
		else if (Optional(Token::Function))
		{
			module->items.push_back(ParseFunction());
		}
		else if (Optional(Token::Class))
		{
			module->items.push_back(ParseClass());
		}
		else
		{
			LogError("Parser::ParseModule", "Unexpected token");
		}
	}

	if(!error_ ) return module;
	return nullptr;
}

std::unique_ptr<NodeAST> Parser::ParseFlowControl()
{
	if (Optional(Token::OpenCurlyBracket))
	{
		return ParseScope();
	}
	if (Optional(Token::If))
	{
		auto if_ast = std::make_unique<IfAST>();
		Expected(Token::OpenRoundBracket);
		if_ast->condition = ParseExpression();
		Expected(Token::CloseRoundBracket);
		if_ast->if_true = ParseFlowControl();
		if (Optional(Token::Else))
		{
			if_ast->otherwise = ParseFlowControl();
		}
		return if_ast;
	}
	if (Optional(Token::For))
	{
		auto for_ast = std::make_unique<ForExprAST>();
		Expected(Token::OpenRoundBracket);
		for_ast->start = Optional(Token::Semicolon) ? nullptr : ParseWholeExpressionLine();
		for_ast->condition = ParseExpression();
		Expected(Token::Semicolon);
		if (!Optional(Token::CloseRoundBracket))
		{
			for_ast->step = ParseExpression();
			Expected(Token::CloseRoundBracket);
		}
		for_ast->body = ParseFlowControl();
		return for_ast;
	}
	if (Optional(Token::Break))
	{
		Expected(Token::Semicolon);
		return std::make_unique<BreakAST>();
	}
	if (Optional(Token::Continue))
	{
		Expected(Token::Semicolon);
		return std::make_unique<ContinueAST>();
	}
	if (Optional(Token::Return))
	{
		auto return_ast = std::make_unique<ReturnAST>();
		if (!Optional(Token::Semicolon))
		{
			return_ast->value = ParseExpression();
			Expected(Token::Semicolon);
		}
		return return_ast;
	}
	//no flow control was found, it must be a regular expression
	std::unique_ptr<ExprAST> expr = ParseWholeExpressionLine();
	return expr;
}

std::unique_ptr<CodeScopeAST> Parser::ParseScope()
{
	auto scope = std::make_unique<CodeScopeAST>();
	while (ShouldContinue())
	{
		if (Optional(Token::CloseCurlyBracket))
		{
			break;
		}
		scope->code.push_back(ParseFlowControl());
	}
	return scope;
}

std::unique_ptr<ExprAST> Parser::ParseWholeExpressionLine()
{
	//TODO: make it not so ugly

	auto ParseTheRemainingExpression = [&](const std::string* already_read_id = nullptr) -> std::unique_ptr<ExprAST>
	{
		auto expression = ParseExpression(already_read_id);
		Expected(Token::Semicolon);
		return expression;
	};

	TypeData type_data;
	TypeParserHelper::TypeParseResult result = TypeParserHelper::OptionalParseType(*this, false, type_data);
	if (TypeParserHelper::TypeParseResult::NotAType == result)
	{
		return ParseTheRemainingExpression();
	}

	VariableData variable_data;
	variable_data.type_data = type_data;
	if (result == TypeParserHelper::TypeParseResult::JustId)
	{
		if(!Optional(Token::Identifier, variable_data.name))
		{ 
			return ParseTheRemainingExpression(&variable_data.type_data.name);
		}
	}
	else if (result == TypeParserHelper::TypeParseResult::Type)
	{
		Expected(Token::Identifier, variable_data.name);
	}
	else
	{
		LogError("Parser::ParseWholeExpressionLine", "Expected a variable name");
		return nullptr;
	}

	auto variable_ast = std::make_unique<LocalVariableDeclarationAST>(variable_data);
	if (Optional(Token::Semicolon))
	{
		return variable_ast;
	}
	std::string operator_str;
	Expected(Token::OperatorExpr, operator_str);
	auto found_op = BinaryOperatorDatabase::Get().FindOperator(operator_str.c_str());
	const bool valid_assign_op = (found_op.op == EBinaryOperator::Assign) || (found_op.op == EBinaryOperator::AssignIfValidChain);
	if (valid_assign_op)
	{
		return std::make_unique<BinaryOpAST>(found_op.op, std::move(variable_ast), ParseTheRemainingExpression());
	}
	LogError("Parser::ParseWholeExpressionLine", "assign operatior expected");
	return nullptr;
}

std::unique_ptr<ExprAST> Parser::ParseExpression(const std::string* already_read_id)
{
	//std::list<std::unique_ptr<ExprAST>> list;
	std::vector<std::unique_ptr<ExprAST>> list;
	bool consume_commas = false;
	int opened_round_brackets = 0;

	auto ContinueExpression = [&]() -> bool
	{
		return ShouldContinue()
			&& (consume_commas || !lexer_.CheckTokenDontProceed(Token::Coma))
			&& ((opened_round_brackets > 0) || !lexer_.CheckTokenDontProceed(Token::CloseRoundBracket))
			&& !lexer_.CheckTokenDontProceed(Token::Semicolon);
	};
	auto ErrorIfFalse = [&](bool value, const char* msg) -> bool
	{
		if (!value)
		{
			LogError("Parser::ParseExpression", msg);
		}
		return value;
	};
	auto HandleIdentifier = [&](std::string id, bool reuse_context) -> std::unique_ptr<ExprAST>
	{
		const bool is_function = lexer_.CheckTokenDontProceed(Token::OpenRoundBracket);
		if (is_function)
		{
			auto call_ast = std::make_unique<CallExprAST>();
			call_ast->function_name = id;
			if (reuse_context)
			{
				call_ast->context = std::move(list.back());
			}
			return call_ast;
		}
		auto variable_ast = std::make_unique<VariableExprAST>();
		variable_ast->name = id;
		if (reuse_context && ErrorIfFalse(!list.empty(), "Expected context"))
		{
			variable_ast->context = std::move(list.back());
		}
		return variable_ast;
	};
	auto AddExpressionToList = [&]()
	{
		std::string str;
		if (Optional(Token::OpenRoundBracket))
		{
			consume_commas = true;
			opened_round_brackets++;
			if (!lexer_.CheckTokenDontProceed(Token::CloseRoundBracket))
			{
				std::unique_ptr<ExprAST> local_ast = ParseExpression();
				ErrorIfFalse(nullptr != local_ast, "Expected ')'");
				CallExprAST* const prev_call_ast = list.empty() ? nullptr : dynamic_cast<CallExprAST*>(list.back().get());
				if (local_ast && prev_call_ast)
				{
					prev_call_ast->args.emplace_back(std::move(local_ast));
				}
				else if (local_ast)
				{
					list.emplace_back(std::move(local_ast));
				}
			}
		}
		else if (Optional(Token::Coma))
		{
			std::unique_ptr<ExprAST> local_ast = ParseExpression();
			CallExprAST* prev_call_ast = list.empty() ? nullptr : dynamic_cast<CallExprAST*>(list.back().get());
			if (ErrorIfFalse(local_ast && prev_call_ast, "Unexpexted ','"))
			{
				prev_call_ast->args.emplace_back(std::move(local_ast));
			}
		}
		else if (Optional(Token::CloseRoundBracket))
		{
			ErrorIfFalse(0 != opened_round_brackets, "Unexpexted ')'");
			consume_commas = false;
			opened_round_brackets--;
		}
		else if (Optional(Token::Dot))
		{
			ExprAST* previous_ast = list.empty() ? nullptr : list.back().get();
			const bool valid_context = dynamic_cast<CallExprAST*>(previous_ast) || dynamic_cast<VariableExprAST*>(previous_ast);
			ErrorIfFalse(valid_context, "Invalid context");

			std::string id;
			Expected(Token::Identifier, id);
			auto new_ast = HandleIdentifier(id, true);
			if (ErrorIfFalse(!list.empty(), "Expected context"))
			{
				list.pop_back();
			}
			list.emplace_back(std::move(new_ast));
		}
		else if (Optional(Token::Identifier, str))
		{
			list.emplace_back(HandleIdentifier(str, false));
		}
		else if (Optional(Token::StringValue, str))
		{
			list.emplace_back(std::make_unique<LiteralStringAST>(str));
		}
		else if (Optional(Token::IntValue, str))
		{
			const int int_val = atoi(str.c_str());
			list.emplace_back(std::make_unique<LiteralIntegerAST>(int_val));
		}
		else if (Optional(Token::FloatValue, str))
		{
			const double double_val = atof(str.c_str());
			list.emplace_back(std::make_unique<LiteralFloatAST>(double_val));
		}
		else if (Optional(Token::OpenSquareBracket))
		{
			const TypeData type_data = ParseType();
			Expected(Token::CloseSquareBracket);
			list.emplace_back(std::make_unique<UnaryOpAST>(EUnaryOperator::Cast, type_data));
		}
		else if (Optional(Token::OperatorExpr, str))
		{
			const BinaryOpAST* const bin_op_ast = list.empty() ? nullptr : dynamic_cast<BinaryOpAST*>(list.back().get());
			const bool do_unary_op = list.empty() || (bin_op_ast && (nullptr == bin_op_ast->rhs));
			if (do_unary_op)
			{
				auto found_unary_operator = UnaryOperatorDatabase::Get().FindOperator(str.c_str());
				ErrorIfFalse(found_unary_operator.op != EUnaryOperator::_Error, str.c_str());
				list.emplace_back(std::make_unique<UnaryOpAST>(found_unary_operator.op));
			}
			else
			{
				auto found_binary_operator = BinaryOperatorDatabase::Get().FindOperator(str.c_str());
				ErrorIfFalse(found_binary_operator.op != EBinaryOperator::_Error, str.c_str());
				list.emplace_back(std::make_unique<BinaryOpAST>(found_binary_operator.op));
			}
		}
		else
		{
			LogError("Parser::ParseExpression unexpected token");
		}
	};

	if (already_read_id)
	{
		list.emplace_back(HandleIdentifier(*already_read_id, false));
	}
	while (ContinueExpression())
	{
		AddExpressionToList();
	}
	ErrorIfFalse(!list.empty(), "Expected an expresion");

	//Apply unary operators - do it in reverse order: op closer to terminal goes first
	for (int ast_iter = list.size() - 2; ast_iter >= 0; --ast_iter)
	{
		ErrorIfFalse(ast_iter < static_cast<int>(list.size()), "Error..");
		UnaryOpAST* const unary_ast = dynamic_cast<UnaryOpAST*>(list[ast_iter].get());
		if (unary_ast)
		{
			const int next_elem = ast_iter + 1;
			unary_ast->terminal = std::move(list[next_elem]);
			list.erase(list.begin() + next_elem);
		}
	}

	//Apply binary operator
	const BinaryOperatorDatabase& database = BinaryOperatorDatabase::Get();
	for (BinaryOperatorDatabase::OperatorData op_data = database.GetHighestPrecedenceOp()
		; op_data.precedence > 0
		; op_data = database.GetNextOpWithLowerPrecedence(op_data.op))
	{
		for (int ast_iter = 1; ast_iter < static_cast<int>(list.size());)
		{
			BinaryOpAST* const binary_ast = dynamic_cast<BinaryOpAST*>(list[ast_iter].get());
			const bool match_precedence = binary_ast && (database.GetOperatorData(binary_ast->opcode).precedence == op_data.precedence);
			ErrorIfFalse(!binary_ast || (!binary_ast->lhs) == (!binary_ast->rhs), "Binary operator was not fully parsed");
			if (match_precedence && !binary_ast->lhs && !binary_ast->rhs)
			{
				const auto prev_iter = ast_iter - 1;
				const auto next_iter = ast_iter + 1;
				binary_ast->lhs = std::move(list[prev_iter]);
				binary_ast->rhs = std::move(list[next_iter]);
				list.erase(list.begin() + next_iter);
				list.erase(list.begin() + prev_iter);
			}
			else
			{
				++ast_iter;
			}
		}
	}
	if (!ErrorIfFalse(list.size() == 1, "Expected single expression"))
	{
		return nullptr;
	}

	return std::move(list.back());
}