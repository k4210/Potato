#pragma once

#include "ast_expression.h"

class ControlFlowAST : public NodeAST
{
};

class CodeScopeAST : public ControlFlowAST
{
public:
	std::vector<std::unique_ptr<NodeAST>> code;

	llvm::Value *codegen() override { return nullptr; }
};

class IfAST : public ControlFlowAST
{
public:
	std::unique_ptr<ExprAST> condition;
	std::unique_ptr<NodeAST> if_true;
	std::unique_ptr<NodeAST> otherwise;

	llvm::Value *codegen() override { return nullptr; }
};

class ForExprAST : public ControlFlowAST 
{
public:
	std::unique_ptr<ExprAST> start, condition, step;
	std::unique_ptr<NodeAST> body;

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
public:
	std::unique_ptr<ExprAST> value;

	llvm::Value *codegen() override { return nullptr; }
};
