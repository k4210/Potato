#include "stdafx.h"

#include "potato_common.h"
#include "ast_expression.h"
#include "ast_flow_control.h"
#include "ast_structure.h"
#include "operator_database.h"
#include "context.h"

llvm::Value* LiteralFloatAST::codegen(Context& context)const 
{
	return llvm::ConstantFP::get(context.the_context_, llvm::APFloat(value_));
}
llvm::Value* LiteralIntegerAST::codegen(Context& context)const
{
	return llvm::ConstantInt::get(context.the_context_, llvm::APInt(sizeof(value_) * 8, value_, true));
}
llvm::Value* LiteralStringAST::codegen(Context& ) const
{
	Utils::SoftAssert(false, "LiteralStringAST::codegen not implemented");
	return nullptr;
}
llvm::Value* LocalVariableDeclarationAST::codegen(Context&)const 
{
	return nullptr;

	//AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
	//Builder.CreateStore(InitVal, Alloca);
}
llvm::Value* VariableExprAST::codegen(Context& context)const 
{
	Utils::SoftAssert(!context_, "LiteralStringAST::codegen not implemented");
	llvm::AllocaInst* alloca_inst = context.FindVariable(name_);
	if (!alloca_inst)
	{
		Utils::LogError("VariableExprAST::codegen", "Unknown variable name");
		return nullptr;
	}

	return context.builder_.CreateLoad(alloca_inst, name_);
}
llvm::Value* UnaryOpAST::codegen(Context& )const { return nullptr; }
llvm::Value* BinaryOpAST::codegen(Context& )const { return nullptr; }
llvm::Value* CallExprAST::codegen(Context& )const { return nullptr; }
llvm::Value* TernaryOpAst::codegen(Context&) const { return nullptr; }

void CodeScopeAST::codegen(Context& )const {}
void IfAST::codegen(Context& )const {}
void ForExprAST::codegen(Context& )const {}
void BreakAST::codegen(Context& )const {}
void ContinueAST::codegen(Context& )const {}
void ReturnAST::codegen(Context& )const {}

void FunctionDeclarationAST::codegen(Context& )const {}
void StructureAST::codegen(Context& )const {}
void ClassAST::codegen(Context& )const {}
void ModuleAST::codegen(Context& )const {}
void ImportAST::codegen(Context& )const {}
