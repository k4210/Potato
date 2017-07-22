#pragma once

#include "potato_common.h"
#include "ast_expression.h"
#include "utils.h"

class ControlFlowAST : public NodeAST
{
};

class CodeScopeAST : public ControlFlowAST
{
public:
	std::vector<std::unique_ptr<NodeAST>> code;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "CodeScopeAST");
		logger.IncreaseIndent();
		for (auto& arg : code)
		{
			arg->log(logger, "line");
		}
		logger.DecreaseIndent();
	}
	void codegen(Context& context) const override;
};

class IfAST : public ControlFlowAST
{
public:
	std::unique_ptr<ExprAST> condition;
	std::unique_ptr<NodeAST> if_true;
	std::unique_ptr<NodeAST> otherwise;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "IfAST");
		logger.IncreaseIndent();
		if (condition)
		{
			condition->log(logger, "condition");
		}
		if (if_true)
		{
			if_true->log(logger, "if_true");
		}
		if (otherwise)
		{
			otherwise->log(logger, "otherwise");
		}
		logger.DecreaseIndent();
	}
	void codegen(Context& context) const override;
};

class ForExprAST : public ControlFlowAST 
{
public:
	std::unique_ptr<ExprAST> start, condition, step;
	std::unique_ptr<NodeAST> body;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "ForExprAST");
		logger.IncreaseIndent();
		if (start)
		{
			start->log(logger, "start");
		}
		if (condition)
		{
			condition->log(logger, "condition");
		}
		if (step)
		{
			step->log(logger, "step");
		}
		if (body)
		{
			body->log(logger, "body");
		}
		logger.DecreaseIndent();
	}
	void codegen(Context& context) const override;
};

class BreakAST : public ControlFlowAST
{
public:
	void codegen(Context& context) const override;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "BreakAST");
	}
};

class ContinueAST : public ControlFlowAST
{
public:
	void codegen(Context& context) const override;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "ContinueAST");
	}
};

class ReturnAST : public ControlFlowAST
{
public:
	std::unique_ptr<ExprAST> value;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "ReturnAST");
		logger.IncreaseIndent();
		if (value)
		{
			value->log(logger, "value");
		}
		logger.DecreaseIndent();
	}
	void codegen(Context& context) const override;
};
