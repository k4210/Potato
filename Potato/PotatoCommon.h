#pragma once

enum class EUnaryOperator
{
	Minus,
	Cast,
	Unknown
};

enum class EBinaryOperator
{
	Assign,
	AssignIfValidChain,
	AddAssign,
	SubAssign,
	MultiplyAssign,
	DivideAssign,
	ModuloAssign,
	AndAssign,
	OrAssign,
	Or,
	And,
	Equal,
	NotEqual,
	GreaterOrEqual,
	LessOrEqual,
	Greater,
	Less,
	Add,
	Sub,
	Multiply,
	Divide,
	Modulo,
	Unknown
};

namespace llvm
{
	class Value;
}

class NodeAST
{
public:
	virtual ~NodeAST() = default;
	virtual llvm::Value *codegen() = 0;
};