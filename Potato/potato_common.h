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

enum class AccessSpecifier
{
	Default,
	Private,
	Public,
};

enum class MutableSpecifier
{
	None		= 0,
	Mutable		= 1 << 0,
	MutableRef	= 1 << 1,
};

struct TypeData
{
	std::string name;
	unsigned int mutable_specifiers;
};

struct VariableData
{
	TypeData type_data;
	std::string name;
	AccessSpecifier access_specifier;
};