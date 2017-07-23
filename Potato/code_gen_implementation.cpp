#include "stdafx.h"

#include "potato_common.h"
#include "ast_expression.h"
#include "ast_flow_control.h"
#include "ast_structure.h"
#include "operator_database.h"
#include "context.h"

void LiteralFloatAST::codegen(Context &)const {}
void LiteralIntegerAST::codegen(Context &)const {}
void LiteralStringAST::codegen(Context &)const {}
void LocalVariableDeclarationAST::codegen(Context &)const {}
void VariableExprAST::codegen(Context &)const {}
void UnaryOpAST::codegen(Context &)const {}
void BinaryOpAST::codegen(Context &)const {}
void CallExprAST::codegen(Context &)const {}
void CodeScopeAST::codegen(Context &)const {}
void IfAST::codegen(Context &)const {}
void ForExprAST::codegen(Context &)const {}
void BreakAST::codegen(Context &)const {}
void ContinueAST::codegen(Context &)const {}
void ReturnAST::codegen(Context &)const {}

void FunctionDeclarationAST::codegen(class Context &)const {}
void StructureAST::codegen(class Context &)const {}
void ClassAST::codegen(class Context &)const {}
void ModuleAST::codegen(class Context &)const {}
void ImportAST::codegen(class Context &)const {}
