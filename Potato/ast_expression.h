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
public:
	const double val;

	LiteralFloatAST(double in_val)
		: val(in_val)
	{}

	llvm::Value* codegen() override { return nullptr; }
};

class LiteralIntegerAST : public ExprAST
{
public:
	const int val;

	LiteralIntegerAST(int in_val)
		: val(in_val)
	{}

	llvm::Value* codegen() override { return nullptr; }
};

class LiteralStringAST : public ExprAST
{
public:
	const std::string val;

	LiteralStringAST(const std::string& in_val)
		: val(in_val)
	{}

	llvm::Value* codegen() override { return nullptr; }
};

class LocalVariableDeclarationAST : public ExprAST
{
public:
	const VariableData variable;

	LocalVariableDeclarationAST(VariableData in_variable)
		: variable(in_variable) //move?
	{}

	llvm::Value *codegen() override { return nullptr; }
};

class VariableExprAST : public ExprAST 
{
public:
	std::string name;
	std::unique_ptr<ExprAST> context;

	llvm::Value *codegen() override { return nullptr; }
};

class UnaryOpAST : public ExprAST 
{
public:
	EUnaryOperator opcode;
	std::unique_ptr<ExprAST> terminal;
	TypeData type_for_cast;

	UnaryOpAST(EUnaryOperator in_opcode)
		: opcode(in_opcode)
	{}

	UnaryOpAST(EUnaryOperator in_opcode, const TypeData& in_type)
		: opcode(in_opcode)
		, type_for_cast(in_type)
	{}

	llvm::Value *codegen() override { return nullptr; }
};

class BinaryOpAST : public ExprAST 
{
public:
	EBinaryOperator opcode;
	std::unique_ptr<ExprAST> lhs, rhs;

	BinaryOpAST(EBinaryOperator in_opcode)
		: opcode(in_opcode) 
	{}

	BinaryOpAST(EBinaryOperator in_opcode, std::unique_ptr<ExprAST> in_lhs, std::unique_ptr<ExprAST> in_rhs)
		: opcode(in_opcode)
		, lhs(std::move(in_lhs))
		, rhs(std::move(in_rhs))
	{}

	llvm::Value *codegen() override { return nullptr; }
};

class CallExprAST : public ExprAST 
{
public:
	std::string function_name;
	std::unique_ptr<ExprAST> context;
	std::vector<std::unique_ptr<ExprAST>> args;

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

