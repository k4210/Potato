#include "lexer.h"
#include "parser.h"
#include "operator_database.h"

void main()
{
	Lexer lexer;
	const BinaryOperatorDatabase& binary_operators = BinaryOperatorDatabase::Get();
	for (const auto iter : binary_operators.GetOperators())
	{
		lexer.RegisterOperatorString(iter.first);
	}

	const UnaryOperatorDatabase& unary_operators = UnaryOperatorDatabase::Get();
	for (const auto iter : unary_operators.GetOperators())
	{
		lexer.RegisterOperatorString(iter.first);
	}

	lexer.Start();
	{
		Parser parser(lexer);
		auto module_stuff = parser.ParseModule();
	}
	lexer.End();
}