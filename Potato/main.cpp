#include "Lexer.h"
#include "OperatorDatabase.h"
#include "AST_Expression.h"
#include "AST_ControlFlow.h"

void main()
{
	Lexer lexer;

	const BinaryOperatorDatabase& binary_operators = BinaryOperatorDatabase::Get();
	for (const auto iter : binary_operators.GetOperators())
	{
		lexer.RegisterOperatorString(iter.first);
	}
}