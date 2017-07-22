#include "stdafx.h"
#include "lexer.h"
#include "parser.h"
#include "operator_database.h"
#include "utils.h"

int main(int argc, char *argv[])
{
	auto WaitForUserToExit = []()
	{
		printf("Press return to exit..");
		getchar();
	};

	if (argc < 2)
	{
		fprintf(stderr, "No file specified!\n");
		WaitForUserToExit();
		return EXIT_FAILURE;
	}

	Lexer lexer;
	for (const auto iter : BinaryOperatorDatabase::Get().GetOperators())
	{
		lexer.RegisterOperatorString(iter.second.name);
	}
	for (const auto iter : UnaryOperatorDatabase::Get().GetOperators())
	{
		lexer.RegisterOperatorString(iter.second.name);
	}

	std::unique_ptr<NodeAST> module_ast;
	{
		fprintf(stderr, "Potato is ready! File: %s\n", argv[1]);
		lexer.Start(argv[1]);
		Parser parser(lexer);
		module_ast = parser.ParseModule();
		lexer.End();
	}

	if (module_ast)
	{
		printf("\n");
		Logger logger;
		module_ast->log(logger, "root");
		printf("\n");
	}
	WaitForUserToExit();
}