#pragma once

#include "utils.h"
#include <memory>

enum class EUnaryOperator
{
	_Error,
	Minus,
	Negation, //!
	ByteNegation, //~
	Cast,
};

enum class EBinaryOperator
{
	_Error,
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
	virtual void log(Logger& logger, const char* contect_str) const = 0;
};

enum class AccessSpecifier
{
	Default,
	Private,
	Public,
};

enum class MutableSpecifier : unsigned int
{
	None		= 0,
	Mutable		= 1 << 0,
	MutableRef	= 1 << 1,
};

enum class EType
{
	Value,
	Reference,
	Void
};

struct TypeData
{
	std::string name;
	unsigned int mutable_specifiers = 0;
	EType type = EType::Value;
	std::string ToString() const
	{
		std::string result;
		if (0 != (mutable_specifiers & static_cast<unsigned int>(MutableSpecifier::MutableRef)))
		{
			result += "mutable_ref ";
		}
		if (0 != (mutable_specifiers & static_cast<unsigned int>(MutableSpecifier::Mutable)))
		{
			result += "mutable ";
		}
		result += name;
		if (type == EType::Reference)
		{
			result += "^";
		}
		return result;
	}
};

struct VariableData
{
	TypeData type_data;
	std::string name;
	AccessSpecifier access_specifier = AccessSpecifier::Default;

	std::string ToString() const
	{
		std::string result;
		if (access_specifier == AccessSpecifier::Private)
		{
			result += "private ";
		}
		if (access_specifier == AccessSpecifier::Public)
		{
			result += "public ";
		}
		result += type_data.ToString();
		result += " ";
		result += name;
		return result;
	}
};