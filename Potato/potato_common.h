#pragma once

#include "utils.h"

enum class EToken
{
	EndOfFile,
	Error,

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
	Int, 
	Float,

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
	class Function;
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

enum class EFunctionMutable : unsigned int
{
	Const,
	Mutable,
};

enum class EVarType
{
	Void		= 0,
	Int			= 1 << 0,
	Float		= 1 << 1,
	ValueStruct	= 1 << 2,
	Reference	= 1 << 3,
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
		switch (type)
		{
			case EVarType::Int: result += "int "; break;
			case EVarType::Float: result += "float "; break;
			case EVarType::ValueStruct: result += "struct "; break;
			case EVarType::Void: result += "void "; break;
			case EVarType::Reference: result += "reference "; break;
		}

		result += name;
		if (type == EVarType::Reference)
		{
			result += "^";
		}
		return result;
	}

	bool Validate() const
	{
		if ((!name.empty()) != (type == EVarType::ValueStruct || type == EVarType::Reference)) return false;
		if (mutable_specifiers.Get(EMutableSpecifier::Mutable) && (type == EVarType::Void)) return false;
		if (mutable_specifiers.Get(EMutableSpecifier::MutableRef) && (type != EVarType::Reference)) return false;
		return true;
	}
};

// Can represent: local variable, function parameter, member variable
struct VariableData
{
	TypeData type_data;
	std::string name;
	EAccessSpecifier access_specifier = EAccessSpecifier::Default;

	llvm::Value* value = nullptr;

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

struct HighLevelEntity
{
	std::string name;
	std::weak_ptr<HighLevelEntity> owner;

};

struct StructData : public HighLevelEntity
{
	std::vector<VariableData> member_fields;
	llvm::Value* value = nullptr;
};

struct FunctionData : public HighLevelEntity
{
	std::vector<VariableData> parameters;

	llvm::Function* function = nullptr;

	TypeData return_type;

	EAccessSpecifier acces_specifier = EAccessSpecifier::Default;
	EFunctionMutable is_mutable = EFunctionMutable::Const;
};

struct ClassData : public StructData
{
	std::string base_class;
	std::vector<std::string> implemented_interfaces;

	std::vector<std::shared_ptr<FunctionData>> functions;
};

struct ModuleData : HighLevelEntity
{
	std::vector<std::shared_ptr<StructData>> structures;
	std::vector<std::shared_ptr<ClassData>> classes;
	std::vector<std::shared_ptr<FunctionData>> functions;

	std::vector<std::weak_ptr<ModuleData>> imported_modules; // ???

	std::unique_ptr<llvm::Module> module_implementation;
};