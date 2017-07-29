#pragma once

#include "potato_common.h"
#include "operator_database.h"
#include "utils.h"

class ExprAST : public NodeAST
{
	virtual llvm::Value* codegen(Context& context) const = 0;
};

class LiteralFloatAST : public ExprAST
{
public:
	const double value_;

	LiteralFloatAST(double value)
		: value_(value)
	{}
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "LiteralFloatAST:", std::to_string(value_).c_str());
	}
	llvm::Value* codegen(Context& context) const override;
};

class LiteralIntegerAST : public ExprAST
{
public:
	const int value_;

	LiteralIntegerAST(int value)
		: value_(value)
	{}
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "LiteralIntegerAST:", std::to_string(value_).c_str());
	}
	llvm::Value* codegen(Context& context) const override;
};

class LiteralStringAST : public ExprAST
{
public:
	const std::string value_;

	LiteralStringAST(const std::string& value)
		: value_(value)
	{}
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "LiteralStringAST:", value_.c_str());
	}
	llvm::Value* codegen(Context& context) const override;
};

class LocalVariableDeclarationAST : public ExprAST
{
public:
	const VariableData variable_;

	LocalVariableDeclarationAST(VariableData variable)
		: variable_(variable) //move?
	{}
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "LocalVariableDeclarationAST:", variable_.ToString().c_str());
	}
	llvm::Value* codegen(Context& context) const override;
};

class VariableExprAST : public ExprAST 
{
public:
	std::string name_;
	std::unique_ptr<ExprAST> context_;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "VariableExprAST:", name_.c_str());
		if (context_)
		{
			logger.IncreaseIndent();
			context_->log(logger, "context");
			logger.DecreaseIndent();
		}
	}
	llvm::Value* codegen(Context& context) const override;
};

class UnaryOpAST : public ExprAST 
{
public:
	EUnaryOperator opcode_;
	std::unique_ptr<ExprAST> terminal_;
	TypeData type_for_cast_;

	UnaryOpAST(EUnaryOperator opcode)
		: opcode_(opcode)
	{}

	UnaryOpAST(EUnaryOperator opcode, const TypeData& type_for_cast)
		: opcode_(opcode)
		, type_for_cast_(type_for_cast)
	{}
	void log(Logger& logger, const char* contect_str) const override
	{
		const OperatorId op_id = UnaryOperatorDatabase::Get().GetOperatorData(opcode_).name;
		logger.PrintLine(contect_str, "UnaryOpAST:", op_id.c_str());
		if (opcode_ == EUnaryOperator::Cast)
		{
			logger.IncreaseIndent();
			logger.PrintLine("type:", type_for_cast_.ToString().c_str());
			logger.DecreaseIndent();
		}
		if (terminal_)
		{
			logger.IncreaseIndent();
			terminal_->log(logger, "terminal");
			logger.DecreaseIndent();
		}
	}
	llvm::Value* codegen(Context& context) const override;
};

class BinaryOpAST : public ExprAST 
{
public:
	EBinaryOperator opcode_;
	std::unique_ptr<ExprAST> lhs_, rhs_;

	BinaryOpAST(EBinaryOperator in_opcode)
		: opcode_(in_opcode)
	{}

	BinaryOpAST(EBinaryOperator in_opcode, std::unique_ptr<ExprAST> in_lhs, std::unique_ptr<ExprAST> in_rhs)
		: opcode_(in_opcode)
		, lhs_(std::move(in_lhs))
		, rhs_(std::move(in_rhs))
	{}
	void log(Logger& logger, const char* contect_str) const override
	{
		const OperatorId op_id = BinaryOperatorDatabase::Get().GetOperatorData(opcode_).name;
		logger.PrintLine(contect_str, "BinaryOpAST:", op_id.c_str());
		logger.IncreaseIndent();
		if (lhs_)
		{
			lhs_->log(logger, "lhs");
		}
		if (rhs_)
		{
			rhs_->log(logger, "rhs");
		}
		logger.DecreaseIndent();
	}
	llvm::Value* codegen(Context& context) const override;
};

class CallExprAST : public ExprAST 
{
public:
	std::string function_name_;
	std::unique_ptr<ExprAST> context_;
	std::vector<std::unique_ptr<ExprAST>> args_;
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "CallExprAST:", function_name_.c_str());
		logger.IncreaseIndent();
		if (context_)
		{
			context_->log(logger, "context");
		}
		for(auto& arg : args_)
		{
			arg->log(logger, "param");
		}
		logger.DecreaseIndent();
	}
	llvm::Value* codegen(Context& context) const override;
};

class TernaryOpAst : public ExprAST
{
public:
	std::unique_ptr<ExprAST> condition_, if_true_, if_false_;

	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "TernaryOpAst");
		logger.IncreaseIndent();
		if (condition_)
		{
			condition_->log(logger, "condition");
		}
		if (if_true_)
		{
			if_true_->log(logger, "if_true");
		}
		if (if_false_)
		{
			if_false_->log(logger, "if_false");
		}
		logger.DecreaseIndent();
	}
	bool IsInitialized() const 
	{
		return condition_ && if_true_ && if_false_;
	}
	llvm::Value* codegen(Context& context) const override;
};

