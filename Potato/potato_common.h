#pragma once

#include "utils.h"

enum class EToken
{
	EndOfFile,

	//keywords
	Class,
	Interface,
	Struct,
	Function,
	This,
	Object,
	Null,
	For,
	Continue,
	Break,
	If,
	Else,
	Return,
	Mutable,
	Mutable_ref,
	Private,
	Public,
	Const,
	Virtual,
	Operator,
	Void,
	True,
	False,
	Static,
	Module,
	Import,
	Enum,
	New,

	// Parantesis, brackets, etc
	OpenCurlyBracket, // {
	CloseCurlyBracket, // }
	OpenRoundBracket, // (
	CloseRoundBracket, // )
	Coma, // ,
	ReferenceSign, // ^
	OpenSquareBracket, // [
	CloseSquareBracket, // ]
	Semicolon, // ;
	Dot, // .
	QuestionMark, // ? 
	Colon, // :

	DoubleColon, // ::
	Arrow, //->
	DoubleDot, // ..

	//The following tokens require a str value

	//literals
	IntValue,
	FloatValue,
	StringValue,

	//other
	OperatorExpr,
	Identifier,
	Error,
};

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

enum class EFunctionType
{
	Global,
	Static,
	Class, //can be virtual
	Struct,//cannot be virtual
	ClassConstructor,
	StructConstructor,
	StructDestructor
};

class Context;
struct Logger;
namespace llvm
{
	class Value;
}

class NodeAST
{
public:
	virtual ~NodeAST() = default;
	virtual void log(Logger& logger, const char* contect_str) const = 0;
};

enum class EAccessSpecifier
{
	Private,
	Default,
	Public,
};

enum class EMutableSpecifier : unsigned int
{
	None		= 0,
	Mutable		= 1 << 0,
	MutableRef	= 1 << 1,
};

enum class EVarType
{
	Int,
	Float,
	ValueStruct,
	Reference,
	Void
};

struct TypeData
{
	std::string name;
	Flag32<EMutableSpecifier> mutable_specifiers;
	EVarType type = EVarType::Void;
	std::string ToString() const
	{
		std::string result;
		if (mutable_specifiers.Get(EMutableSpecifier::MutableRef))
		{
			result += "mutable_ref ";
		}
		if (mutable_specifiers.Get(EMutableSpecifier::Mutable))
		{
			result += "mutable ";
		}
		result += name;
		if (type == EVarType::Reference)
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
	EAccessSpecifier access_specifier = EAccessSpecifier::Default;

	std::string ToString() const
	{
		std::string result;
		if (access_specifier == EAccessSpecifier::Private)
		{
			result += "private ";
		}
		if (access_specifier == EAccessSpecifier::Public)
		{
			result += "public ";
		}
		result += type_data.ToString();
		result += " ";
		result += name;
		return result;
	}
};