#pragma once

#include "potato_common.h"
#include "ast_expression.h"
#include "utils.h"

class ControlFlowAST : public NodeAST
{
public:
	virtual void Codegen(Context& context) const = 0;
};

class CodeScopeAST : public ControlFlowAST
{
public:
	std::vector<std::unique_ptr<NodeAST>> code_;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "CodeScopeAST");
		logger.IncreaseIndent();
		for (auto& arg : code_)
		{
			arg->log(logger, "line");
		}
		logger.DecreaseIndent();
	}
	void Codegen(Context& context) const override;
};

class IfAST : public ControlFlowAST
{
public:
	std::unique_ptr<ExprAST> condition_;
	std::unique_ptr<NodeAST> if_true_;
	std::unique_ptr<NodeAST> otherwise_;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "IfAST");
		logger.IncreaseIndent();
		if (condition_)
		{
			condition_->log(logger, "condition");
		}
		if (if_true_)
		{
			if_true_->log(logger, "if_true");
		}
		if (otherwise_)
		{
			otherwise_->log(logger, "otherwise");
		}
		logger.DecreaseIndent();
	}
	void Codegen(Context& context) const override;
};

class ForExprAST : public ControlFlowAST 
{
public:
	std::unique_ptr<ExprAST> start_, condition_, step_;
	std::unique_ptr<NodeAST> body_;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "ForExprAST");
		logger.IncreaseIndent();
		if (start_)
		{
			start_->log(logger, "start");
		}
		if (condition_)
		{
			condition_->log(logger, "condition");
		}
		if (step_)
		{
			step_->log(logger, "step");
		}
		if (body_)
		{
			body_->log(logger, "body");
		}
		logger.DecreaseIndent();
	}
	void Codegen(Context& context) const override;
};

class BreakAST : public ControlFlowAST
{
public:
	void Codegen(Context& context) const override;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "BreakAST");
	}
};

class ContinueAST : public ControlFlowAST
{
public:
	void Codegen(Context& context) const override;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "ContinueAST");
	}
};

class ReturnAST : public ControlFlowAST
{
public:
	std::unique_ptr<ExprAST> value_;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "ReturnAST");
		logger.IncreaseIndent();
		if (value_)
		{
			value_->log(logger, "value");
		}
		logger.DecreaseIndent();
	}
	void Codegen(Context& context) const override;
};
