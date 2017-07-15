#pragma once

#include "potato_common.h"
#include <string>
#include <memory>
#include <vector>

class ExprAST : public NodeAST
{
	// it suppose to return a value (or call a void function)
};

class LiteralFloatAST : public ExprAST
{
	double val;

public:
	LiteralFloatAST(double in_val)
		: val(in_val)
	{}

	llvm::Value* codegen() override { return nullptr; }
};

class LiteralIntegerAST : public ExprAST
{
	int val;

public:
	LiteralIntegerAST(int in_val)
		: val(in_val)
	{}

	llvm::Value* codegen() override { return nullptr; }
};

class LiteralStringAST : public ExprAST
{
	std::string val;

public:
	LiteralStringAST(const std::string& in_val)
		: val(in_val)
	{}

	llvm::Value* codegen() override { return nullptr; }
};

class LocalVariableDeclarationAST : public ExprAST
{
	VariableData variable;

public:
	LocalVariableDeclarationAST(VariableData in_variable)
		: variable(in_variable) //move?
	{}

	llvm::Value *codegen() override { return nullptr; }
	const VariableData& GetName() const { return variable; }
};

class TerminalExprAST : public ExprAST 
{
	std::string name;
	std::unique_ptr<ExprAST> context;
public:
	TerminalExprAST(const std::string &in_name, std::unique_ptr<ExprAST> in_context)
		: name(in_name) 
		, context(std::move(in_context))
	{}

	llvm::Value *codegen() override { return nullptr; }
	const std::string& GetName() const { return name; }
};

class UnaryOpAST : public ExprAST 
{
	EUnaryOperator opcode;
	std::unique_ptr<ExprAST> terminal;
	//Type for cast

public:
	UnaryOpAST(EUnaryOperator in_opcode, std::unique_ptr<ExprAST> in_terminal)
		: opcode(in_opcode)
		, terminal(std::move(in_terminal)) 
	{}

	llvm::Value *codegen() override { return nullptr; }
};

class BinaryOpAST : public ExprAST 
{
	EBinaryOperator opcode;
	std::unique_ptr<ExprAST> lhs, rhs;

public:
	BinaryOpAST(EBinaryOperator in_opcode, std::unique_ptr<ExprAST> in_lhs, std::unique_ptr<ExprAST> in_rhs)
		: opcode(in_opcode)
		, lhs(std::move(in_lhs))
		, rhs(std::move(in_rhs)) {}

	llvm::Value *codegen() override;
};

class CallExprAST : public ExprAST 
{
	std::string function_name;
	std::unique_ptr<ExprAST> context;
	std::vector<std::unique_ptr<ExprAST>> args;

public:
	CallExprAST(const std::string &in_function, std::unique_ptr<ExprAST> in_context, std::vector<std::unique_ptr<ExprAST>> in_args)
		: function_name(in_function)
		, context(std::move(in_context))
		, args(std::move(in_args)) {}

	llvm::Value *codegen() override { return nullptr; }
};

class TernaryOpAst : public ExprAST
{
	std::unique_ptr<ExprAST> condition, if_true, if_false;

public:
	TernaryOpAst(std::unique_ptr<ExprAST> in_condition, std::unique_ptr<ExprAST> in_if_true, std::unique_ptr<ExprAST> in_if_false)
		: condition(std::move(in_condition))
		, if_true(std::move(in_if_true))
		, if_false(std::move(in_if_false))
	{}

	llvm::Value *codegen() override { return nullptr; }
};

