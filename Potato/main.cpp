#include "lexer.h"
#include "operator_database.h"
#include "ast_expression.h"
#include "ast_flow_control.h"

void main()
{
	Lexer lexer;

	const BinaryOperatorDatabase& binary_operators = BinaryOperatorDatabase::Get();
	for (const auto iter : binary_operators.GetOperators())
	{
		lexer.RegisterOperatorString(iter.first);
	}
}