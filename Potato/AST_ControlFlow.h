#pragma once

#include "AST_Expression.h"

class ControlFlowAST : public NodeAST
{
};

class CodeScopeAST : public ControlFlowAST
{
	// variables
	// control flow
	llvm::Value *codegen() override { return nullptr; }
};

class IfAST : public ControlFlowAST
{
	std::unique_ptr<ExprAST> condition;
	std::unique_ptr<CodeScopeAST> if_true;
	std::unique_ptr<ControlFlowAST> otherwise;
public:
	IfAST(std::unique_ptr<ExprAST> in_condition, std::unique_ptr<CodeScopeAST> in_if_true, std::unique_ptr<ExprAST> in_otherwise)
		: condition(std::move(in_condition))
		, if_true(std::move(in_if_true))
		, otherwise(std::move(otherwise)) 
	{}

	llvm::Value *codegen() override { return nullptr; }
};

class ForExprAST : public ControlFlowAST {
	std::unique_ptr<ExprAST> start, end, step;
	std::unique_ptr<CodeScopeAST> body;

public:
	ForExprAST(std::unique_ptr<ExprAST> in_start, std::unique_ptr<ExprAST> in_end, std::unique_ptr<ExprAST> in_step, std::unique_ptr<CodeScopeAST> in_body)
		: start(std::move(in_start))
		, end(std::move(in_end))
		, step(std::move(in_step))
		, body(std::move(in_body)) 
	{}

	llvm::Value *codegen() override { return nullptr; }
};

class BreakAST : public ControlFlowAST
{
	llvm::Value *codegen() override { return nullptr; }
};

class ContinueAST : public ControlFlowAST
{
	llvm::Value *codegen() override { return nullptr; }
};

class ReturnAST : public ControlFlowAST
{
	std::unique_ptr<ExprAST> value;
	ReturnAST(std::unique_ptr<ExprAST> in_value)
		: value(std::move(in_value))
	{}
	llvm::Value *codegen() override { return nullptr; }
};

class HighLevelAST
{
public:
	virtual ~HighLevelAST() = default;
	virtual llvm::Value *codegen() = 0;
};
 
class ClassAST : public HighLevelAST
{
	llvm::Value *codegen() override { return nullptr; }
};

class FunctionDeclarationAST : public HighLevelAST
{
	llvm::Value *codegen() override { return nullptr; }
};

class FunctionImplementationAST : public HighLevelAST
{
	llvm::Value *codegen() override { return nullptr; }
};
//class declaration
//Function declaration
//structure declaration
// member field declaration

//function definition
