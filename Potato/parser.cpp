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
	msg1 = msg1 ? msg1 : "Parser Error";
	error_ = true;
	Utils::LogError(lexer_.GetCodeLocation().c_str(), msg1, msg2);
}

bool Parser::Optional(EToken token)
{
	return lexer_.Consume(token);
}
bool Parser::Optional(EToken token, std::string& out_str)
{
	return lexer_.Consume(token, out_str);
}

bool Parser::Expected(EToken expected_token, const char* error_msg)
{
	if (!error_)
	{
		if (lexer_.Consume(expected_token))
		{
			return true;
		}
		LogError(error_msg);
	}
	return false;
}

bool Parser::Expected(EToken expected_token, std::string& out_str, const char* error_msg)
{
	if (!error_)
	{
		if (lexer_.Consume(expected_token, out_str))
		{
			return true;
		}
		LogError(error_msg);
	}
	return false;
}

bool Parser::ShouldContinue()
{
	error_ |= lexer_.Consume(EToken::Error);
	eof_ |= lexer_.Consume(EToken::EndOfFile);

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
		if (allow_void && parser.Optional(EToken::Void))
		{
			out_type.type = EVarType::Void;
			return TypeParseResult::Type;
		}

		TypeParseResult result = TypeParseResult::Unknown;
		while (parser.ShouldContinue())
		{
			if (parser.Optional(EToken::Mutable))
			{
				out_type.mutable_specifiers.Add(EMutableSpecifier::Mutable);
				result = TypeParseResult::Type;
			}
			else if (parser.Optional(EToken::Mutable_ref))
			{
				out_type.mutable_specifiers.Add(EMutableSpecifier::MutableRef);
				result = TypeParseResult::Type;
			}
			else
			{
				break;
			}
		}

		if (parser.Optional(EToken::Int, out_type.name))
		{
			out_type.type = EVarType::Int;
			result = TypeParseResult::Type;
		}
		else if (parser.Optional(EToken::Float, out_type.name))
		{
			out_type.type = EVarType::Float;
			result = TypeParseResult::Type;
		}
		else if (!parser.Optional(EToken::Identifier, out_type.name))
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
		else if (TypeParseResult::Type == result)
		{
			out_type.type = EVarType::ValueStruct;
		}

		if (parser.Optional(EToken::ReferenceSign))
		{
			out_type.type = EVarType::Reference;
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
	if(result == TypeParserHelper::TypeParseResult::JustId)
	{
		type_data.type = EVarType::ValueStruct;
	}
	if (!valid_type || !type_data.Validate())
	{
		LogError("Parser::ParseType", "Expected a type");
	}
	return type_data;
}

EAccessSpecifier Parser::ParseOptionalAccessSpecifier()
{
	if (Optional(EToken::Public))
	{
		return EAccessSpecifier::Public;
	}
	else if (Optional(EToken::Private))
	{
		return EAccessSpecifier::Private;
	}
	return EAccessSpecifier::Default;
}

VariableData Parser::ParseMemberField()
{
	VariableData data;
	data.access_specifier = ParseOptionalAccessSpecifier();
	data.type_data = ParseType();

	Expected(EToken::Identifier, data.name, "Expected a variable name");
	Expected(EToken::Semicolon, "expected ;");

	return data;
}

std::unique_ptr<StructureAST> Parser::ParseStruct()
{
	auto struct_data = std::make_shared<StructData>();
	Expected(EToken::Identifier, struct_data->name, "Expected struct name");
	Expected(EToken::OpenCurlyBracket, "Expected open scope");
	while(ShouldContinue())
	{
		if (Optional(EToken::CloseCurlyBracket))
		{
			Optional(EToken::Semicolon);
			break;
		}
		auto member_field = ParseMemberField();
		struct_data->member_fields.emplace(member_field.name, member_field);
	}

	return std::make_unique<StructureAST>(struct_data);
}

std::unique_ptr<FunctionDeclarationAST> Parser::ParseFunction()
{
	auto function_data = std::make_shared<FunctionData>();
	function_data->acces_specifier = ParseOptionalAccessSpecifier();
	function_data->return_type = ParseType(true);
	Expected(EToken::Identifier, function_data->name, "Expected name");
	Expected(EToken::OpenRoundBracket, "Expected (");
	if (!Optional(EToken::CloseRoundBracket))
	{
		while (ShouldContinue())
		{
			VariableData parameter;
			parameter.type_data = ParseType();
			Expected(EToken::Identifier, parameter.name, "Expected parameter name");
			function_data->parameters.push_back(parameter);
			if (Optional(EToken::CloseRoundBracket))
			{
				break;
			}
			Expected(EToken::Coma, "Expecter ')' or ','");
		}
	}
	function_data->is_mutable = Optional(EToken::Mutable) ? EFunctionMutable::Mutable : EFunctionMutable::Const;

	std::unique_ptr<FunctionDeclarationAST> function_ast = std::make_unique<FunctionDeclarationAST>(function_data);
	if (!Optional(EToken::Semicolon))
	{
		Expected(EToken::OpenCurlyBracket, "Expected {");
		function_ast->optional_body_ = ParseScope();
	}
	return function_ast;
}

std::unique_ptr<ClassAST> Parser::ParseClass()
{
	auto class_data = std::make_shared<ClassData>();
	Expected(EToken::Identifier, class_data->name, "Expected struct name");
	if (Optional(EToken::Colon))
	{
		Expected(EToken::Identifier, class_data->base_class, "Expected base struct name");
	}
	Expected(EToken::OpenCurlyBracket, "Expected open scope");

	std::unique_ptr<ClassAST> class_ast = std::make_unique<ClassAST>(class_data);
	while (ShouldContinue())
	{
		if (Optional(EToken::CloseCurlyBracket))
		{
			Optional(EToken::Semicolon);
			break;
		}
		if (Optional(EToken::Function))
		{
			class_ast->functions_.push_back(ParseFunction());
		}
		else
		{
			auto member_field = ParseMemberField();
			class_data->member_fields.emplace(member_field.name, member_field);
		}
	}
	class_ast->BindParsedChildren();
	return class_ast;
}

std::unique_ptr<ModuleAST> Parser::ParseModule()
{
	auto module_data = std::make_shared<ModuleData>();
	if (Optional(EToken::Module))
	{
		Expected(EToken::Identifier, module_data->name, "Expected module name");
		Expected(EToken::Semicolon, "Expected ';'");
	}

	std::unique_ptr<ModuleAST> module = std::make_unique<ModuleAST>(module_data);
	while (ShouldContinue())
	{
		std::string identifier;
		if (Optional(EToken::Import))
		{
			Expected(EToken::Identifier, identifier, "Expected module name");
			module->imports_.push_back(std::make_unique<ImportAST>(identifier));
			Expected(EToken::Semicolon, "Expected ';'");
		}
		else if (Optional(EToken::Struct))
		{
			module->structures_.push_back(ParseStruct());
		}
		else if (Optional(EToken::Function))
		{
			module->functions_.push_back(ParseFunction());
		}
		else if (Optional(EToken::Class))
		{
			module->classes_.push_back(ParseClass());
		}
		else
		{
			LogError("Parser::ParseModule", "Unexpected token");
		}
	}
	module->BindParsedChildren();
	if(!error_ ) return module;
	return nullptr;
}

std::unique_ptr<NodeAST> Parser::ParseFlowControl()
{
	if (Optional(EToken::OpenCurlyBracket))
	{
		return ParseScope();
	}
	if (Optional(EToken::If))
	{
		auto if_ast = std::make_unique<IfAST>();
		Expected(EToken::OpenRoundBracket);
		if_ast->condition_ = ParseExpression();
		Expected(EToken::CloseRoundBracket);
		if_ast->if_true_ = ParseFlowControl();
		if (Optional(EToken::Else))
		{
			if_ast->otherwise_ = ParseFlowControl();
		}
		return if_ast;
	}
	if (Optional(EToken::For))
	{
		auto for_ast = std::make_unique<ForExprAST>();
		Expected(EToken::OpenRoundBracket);
		for_ast->start_ = Optional(EToken::Semicolon) ? nullptr : ParseWholeExpressionLine();
		for_ast->condition_ = ParseExpression();
		Expected(EToken::Semicolon);
		if (!Optional(EToken::CloseRoundBracket))
		{
			for_ast->step_ = ParseExpression();
			Expected(EToken::CloseRoundBracket);
		}
		for_ast->body_ = ParseFlowControl();
		return for_ast;
	}
	if (Optional(EToken::Break))
	{
		Expected(EToken::Semicolon);
		return std::make_unique<BreakAST>();
	}
	if (Optional(EToken::Continue))
	{
		Expected(EToken::Semicolon);
		return std::make_unique<ContinueAST>();
	}
	if (Optional(EToken::Return))
	{
		auto return_ast = std::make_unique<ReturnAST>();
		if (!Optional(EToken::Semicolon))
		{
			return_ast->value_ = ParseExpression();
			Expected(EToken::Semicolon);
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
		if (Optional(EToken::CloseCurlyBracket))
		{
			break;
		}
		scope->code_.push_back(ParseFlowControl());
	}
	return scope;
}

std::unique_ptr<ExprAST> Parser::ParseWholeExpressionLine()
{
	//TODO: make it not so ugly

	auto ParseTheRemainingExpression = [&](const std::string* already_read_id = nullptr) -> std::unique_ptr<ExprAST>
	{
		auto expression = ParseExpression(already_read_id);
		Expected(EToken::Semicolon);
		return expression;
	};

	TypeData type_data;
	const TypeParserHelper::TypeParseResult result = TypeParserHelper::OptionalParseType(*this, false, type_data);
	if (TypeParserHelper::TypeParseResult::NotAType == result)
	{
		return ParseTheRemainingExpression();
	}

	VariableData variable_data;
	variable_data.type_data = type_data;
	if (result == TypeParserHelper::TypeParseResult::JustId)
	{
		if(!Optional(EToken::Identifier, variable_data.name))
		{ 
			return ParseTheRemainingExpression(&variable_data.type_data.name);
		}
	}
	else if (result == TypeParserHelper::TypeParseResult::Type)
	{
		if (!variable_data.type_data.Validate())
		{
			LogError("Parser::ParseWholeExpressionLine", "Invalid variable type");
		}
		Expected(EToken::Identifier, variable_data.name);
	}
	else
	{
		LogError("Parser::ParseWholeExpressionLine", "Expected a variable name");
		return nullptr;
	}

	auto variable_ast = std::make_unique<LocalVariableDeclarationAST>(variable_data);
	if (Optional(EToken::Semicolon))
	{
		return variable_ast;
	}
	std::string operator_str;
	Expected(EToken::OperatorExpr, operator_str);
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
	auto GetListSize = [&]() -> int
	{
		return static_cast<int>(list.size());
	};
	bool consume_commas = false;
	int opened_round_brackets = 0;

	auto ContinueExpression = [&]() -> bool
	{
		return ShouldContinue()
			&& (consume_commas || !lexer_.CheckTokenDontProceed(EToken::Coma))
			&& ((opened_round_brackets > 0) || !lexer_.CheckTokenDontProceed(EToken::CloseRoundBracket))
			&& !lexer_.CheckTokenDontProceed(EToken::Semicolon);
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
		const bool is_function = lexer_.CheckTokenDontProceed(EToken::OpenRoundBracket);
		if (is_function)
		{
			auto call_ast = std::make_unique<CallExprAST>();
			call_ast->function_name_ = id;
			if (reuse_context)
			{
				call_ast->context_ = std::move(list.back());
			}
			return call_ast;
		}
		auto variable_ast = std::make_unique<VariableExprAST>();
		variable_ast->name = id;
		if (reuse_context && ErrorIfFalse(!list.empty(), "Expected context"))
		{
			variable_ast->context_ = std::move(list.back());
		}
		return variable_ast;
	};
	class TempSecondTernaryOpAst : public ExprAST
	{
	public:
		TernaryOpAst* primary_ast_ = nullptr;

		TempSecondTernaryOpAst(TernaryOpAst* primary_ast)
			: primary_ast_(primary_ast)
		{}

		void log(Logger& logger, const char* contect_str) const override
		{
			logger.PrintLine(contect_str, "TempSecondTernaryOpAst");
		}
		llvm::Value* Codegen(Context&) const override
		{
			Utils::SoftAssert(false, "TempSecondTernaryOpAst cannot generate code. It should be consumed by the Parser");
			return false;
		}
	};
	auto AddExpressionToList = [&]()
	{
		std::string str;
		if (Optional(EToken::OpenRoundBracket))
		{
			consume_commas = true;
			opened_round_brackets++;
			if (!lexer_.CheckTokenDontProceed(EToken::CloseRoundBracket))
			{
				std::unique_ptr<ExprAST> local_ast = ParseExpression();
				ErrorIfFalse(nullptr != local_ast, "Expected ')'");
				CallExprAST* const prev_call_ast = list.empty() ? nullptr : dynamic_cast<CallExprAST*>(list.back().get());
				if (local_ast && prev_call_ast)
				{
					prev_call_ast->args_.emplace_back(std::move(local_ast));
				}
				else if (local_ast)
				{
					list.emplace_back(std::move(local_ast));
				}
			}
		}
		else if (Optional(EToken::Coma))
		{
			std::unique_ptr<ExprAST> local_ast = ParseExpression();
			CallExprAST* prev_call_ast = list.empty() ? nullptr : dynamic_cast<CallExprAST*>(list.back().get());
			if (ErrorIfFalse(local_ast && prev_call_ast, "Unexpexted ','"))
			{
				prev_call_ast->args_.emplace_back(std::move(local_ast));
			}
		}
		else if (Optional(EToken::CloseRoundBracket))
		{
			ErrorIfFalse(0 != opened_round_brackets, "Unexpexted ')'");
			consume_commas = false;
			opened_round_brackets--;
		}
		else if (Optional(EToken::Dot))
		{
			ExprAST* previous_ast = list.empty() ? nullptr : list.back().get();
			const bool valid_context = dynamic_cast<CallExprAST*>(previous_ast) || dynamic_cast<VariableExprAST*>(previous_ast);
			ErrorIfFalse(valid_context, "Invalid context");

			std::string id;
			Expected(EToken::Identifier, id);
			auto new_ast = HandleIdentifier(id, true);
			if (ErrorIfFalse(!list.empty(), "Expected context"))
			{
				list.pop_back();
			}
			list.emplace_back(std::move(new_ast));
		}
		else if (Optional(EToken::QuestionMark))
		{
			list.emplace_back(std::make_unique<TernaryOpAst>());
		}
		else if (Optional(EToken::Colon))
		{
			auto found_it = std::find_if(list.rbegin(), list.rend(), [](const std::unique_ptr<ExprAST>& expr_ast)
			{
				return nullptr != dynamic_cast<TernaryOpAst*>(expr_ast.get());
			});
			if (ErrorIfFalse(found_it != list.rend(), "Missing ? before :"))
			{
				list.emplace_back(std::make_unique<TempSecondTernaryOpAst>(dynamic_cast<TernaryOpAst*>(found_it->get())));
			}
		}
		else if (Optional(EToken::Identifier, str))
		{
			list.emplace_back(HandleIdentifier(str, false));
		}
		else if (Optional(EToken::StringValue, str))
		{
			list.emplace_back(std::make_unique<LiteralStringAST>(str));
		}
		else if (Optional(EToken::IntValue, str))
		{
			const int int_val = atoi(str.c_str());
			list.emplace_back(std::make_unique<LiteralIntegerAST>(int_val));
		}
		else if (Optional(EToken::FloatValue, str))
		{
			const double double_val = atof(str.c_str());
			list.emplace_back(std::make_unique<LiteralFloatAST>(double_val));
		}
		else if (Optional(EToken::OpenSquareBracket))
		{
			const TypeData type_data = ParseType();
			Expected(EToken::CloseSquareBracket);
			list.emplace_back(std::make_unique<UnaryOpAST>(EUnaryOperator::Cast, type_data));
		}
		else if (Optional(EToken::OperatorExpr, str))
		{
			bool do_unary_op = list.empty();
			if (!do_unary_op)
			{
				const BinaryOpAST* const bin_op_ast = dynamic_cast<BinaryOpAST*>(list.back().get());
				const TernaryOpAst* const ternary_op_ast = dynamic_cast<TernaryOpAst*>(list.back().get());
				const TempSecondTernaryOpAst* const temp_ternary_op_ast = dynamic_cast<TempSecondTernaryOpAst*>(list.back().get());
				const bool empty_binary_op = bin_op_ast && (nullptr == bin_op_ast->rhs_);
				const bool empty_ternary_op = temp_ternary_op_ast || (ternary_op_ast && !ternary_op_ast->IsInitialized());
				do_unary_op = empty_binary_op || empty_ternary_op;
			}
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
		ErrorIfFalse(ast_iter < GetListSize(), "Error..");
		UnaryOpAST* const unary_ast = dynamic_cast<UnaryOpAST*>(list[ast_iter].get());
		if (unary_ast)
		{
			const int next_elem = ast_iter + 1;
			unary_ast->terminal_ = std::move(list[next_elem]);
			list.erase(list.begin() + next_elem);
		}
	}

	// Binary op
	const BinaryOperatorDatabase& database = BinaryOperatorDatabase::Get();
	auto ApplyBinaryOperatorWithCommonPrecedence = [&](int precedence)
	{
		for (int ast_iter = 1; ast_iter < GetListSize(); ast_iter++)
		{
			BinaryOpAST* const binary_ast = dynamic_cast<BinaryOpAST*>(list[ast_iter].get());
			const bool match_precedence = binary_ast && (database.GetOperatorData(binary_ast->opcode_).precedence == precedence);
			ErrorIfFalse(!binary_ast || (!binary_ast->lhs_) == (!binary_ast->rhs_), "Binary operator was not fully parsed");
			if (match_precedence && !binary_ast->lhs_ && !binary_ast->rhs_)
			{
				const auto prev_iter = ast_iter - 1;
				const auto next_iter = ast_iter + 1;
				binary_ast->lhs_ = std::move(list[prev_iter]);
				binary_ast->rhs_ = std::move(list[next_iter]);
				list.erase(list.begin() + next_iter);
				list.erase(list.begin() + prev_iter);
				--ast_iter;
			}
		}
	};
	const auto assigment_op = database.GetOperatorData(EBinaryOperator::Assign);

	//Apply binary operator #1
	for (auto op_data = database.GetHighestPrecedenceOp(); op_data.precedence > assigment_op.precedence
		; op_data = database.GetNextOpWithLowerPrecedence(op_data.op))
	{
		ApplyBinaryOperatorWithCommonPrecedence(op_data.precedence);
	}
	//Ternary Op - just one per expression
	for (int ast_iter = 1; ast_iter < GetListSize(); ++ast_iter)
	{
		TernaryOpAst* const ternary_ast = dynamic_cast<TernaryOpAst*>(list[ast_iter].get());
		if (!ternary_ast) continue;
		const int exepected_index_condition = ast_iter - 1;
		const int exepected_index_if_true = ast_iter + 1;
		const int exepected_index_second_op = ast_iter + 2;
		const int exepected_index_if_false = ast_iter + 3;
		bool secondary_op_found = false;
		{
			auto temp_ast = (exepected_index_if_false < GetListSize()) 
				? dynamic_cast<TempSecondTernaryOpAst*>(list[exepected_index_second_op].get()) : nullptr;
			secondary_op_found = temp_ast && temp_ast->primary_ast_ == ternary_ast;
		}
		if (ErrorIfFalse(secondary_op_found, "Missing ':' or the other expression"))
		{
			ternary_ast->condition_ = std::move(list[exepected_index_condition]);
			ternary_ast->if_true_ = std::move(list[exepected_index_if_true]);
			ternary_ast->if_false_ = std::move(list[exepected_index_if_false]);
			list.erase(list.begin() + exepected_index_if_true, list.begin() + exepected_index_if_false + 1);
			list.erase(list.begin() + exepected_index_condition);
			break;
		}
	}

	//Apply binary operator #1
	for (auto op_data = assigment_op; op_data.precedence > 0
		; op_data = database.GetNextOpWithLowerPrecedence(op_data.op))
	{
		ApplyBinaryOperatorWithCommonPrecedence(op_data.precedence);
	}

	if (!ErrorIfFalse(list.size() == 1, "Expected single expression"))
	{
		return nullptr;
	}

	return std::move(list.back());
}