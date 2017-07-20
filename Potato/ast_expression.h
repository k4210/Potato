#pragma once

#include "potato_common.h"
#include <string>
#include <memory>
#include <vector>
#include "operator_database.h"

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
	void log(Logger& logger, const char* contect_str) const override
	{
		char buff[512];
		sprintf_s(buff, "LiteralFloatAST %f", val);
		logger.PrintLine(contect_str, buff);
	}
	llvm::Value* codegen() override { return nullptr; }
};

class LiteralIntegerAST : public ExprAST
{
public:
	const int val;

	LiteralIntegerAST(int in_val)
		: val(in_val)
	{}
	void log(Logger& logger, const char* contect_str) const override
	{
		char buff[512];
		sprintf_s(buff, "LiteralIntegerAST %i", val);
		logger.PrintLine(contect_str, buff);
	}
	llvm::Value* codegen() override { return nullptr; }
};

class LiteralStringAST : public ExprAST
{
public:
	const std::string val;

	LiteralStringAST(const std::string& in_val)
		: val(in_val)
	{}
	void log(Logger& logger, const char* contect_str) const override
	{
		char buff[1024];
		sprintf_s(buff, "LiteralStringAST \"%s\"", val.c_str());
		logger.PrintLine(contect_str, buff);
	}
	llvm::Value* codegen() override { return nullptr; }
};

class LocalVariableDeclarationAST : public ExprAST
{
public:
	const VariableData variable;

	LocalVariableDeclarationAST(VariableData in_variable)
		: variable(in_variable) //move?
	{}
	void log(Logger& logger, const char* contect_str) const override
	{
		char buff[1024];
		sprintf_s(buff, "LocalVariableDeclarationAST %s", variable.ToString().c_str());
		logger.PrintLine(contect_str, buff);
	}
	llvm::Value *codegen() override { return nullptr; }
};

class VariableExprAST : public ExprAST 
{
public:
	std::string name;
	std::unique_ptr<ExprAST> context;
	void log(Logger& logger, const char* contect_str) const override
	{
		char buff[1024];
		sprintf_s(buff, "VariableExprAST %s", name.c_str());
		logger.PrintLine(contect_str, buff);
		if (context)
		{
			logger.IncreaseIndent();
			context->log(logger, "context");
			logger.DecreaseIndent();
		}
	}
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
	void log(Logger& logger, const char* contect_str) const override
	{
		char buff[1024];
		const OperatorId op_id = UnaryOperatorDatabase::Get().GetOperatorData(opcode).name;
		std::string type_str;
		if (opcode == EUnaryOperator::Cast)
		{
			type_str = type_for_cast.ToString();
		}
		sprintf_s(buff, "UnaryOpAST %s %s", op_id.c_str(), type_str.c_str());
		logger.PrintLine(contect_str, buff);
		if (terminal)
		{
			logger.IncreaseIndent();
			terminal->log(logger, "terminal");
			logger.DecreaseIndent();
		}
	}
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
	void log(Logger& logger, const char* contect_str) const override
	{
		char buff[1024];
		const OperatorId op_id = BinaryOperatorDatabase::Get().GetOperatorData(opcode).name;
		sprintf_s(buff, "BinaryOpAST %s", op_id.c_str());
		logger.PrintLine(contect_str, buff);
		logger.IncreaseIndent();
		if (lhs)
		{
			lhs->log(logger, "lhs");
		}
		if (rhs)
		{
			rhs->log(logger, "rhs");
		}
		logger.DecreaseIndent();
	}
	llvm::Value *codegen() override { return nullptr; }
};

class CallExprAST : public ExprAST 
{
public:
	std::string function_name;
	std::unique_ptr<ExprAST> context;
	std::vector<std::unique_ptr<ExprAST>> args;
	void log(Logger& logger, const char* contect_str) const override
	{
		char buff[1024];
		sprintf_s(buff, "CallExprAST %s", function_name.c_str());
		logger.PrintLine(contect_str, buff);
		logger.IncreaseIndent();
		if (context)
		{
			context->log(logger, "context");
		}
		for(auto& arg : args)
		{
			arg->log(logger, "param");
		}
		logger.DecreaseIndent();
	}
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
	void log(Logger& logger, const char* contect_str) const override
	{
		logger.PrintLine(contect_str, "TernaryOpAst");
	}
	llvm::Value *codegen() override { return nullptr; }
};

