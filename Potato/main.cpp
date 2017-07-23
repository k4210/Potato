#include "stdafx.h"

#include "lexer.h"
#include "parser.h"
#include "operator_database.h"
#include "utils.h"
#include "ast_structure.h"

bool CodeGen(ModuleAST* module_root);

void InitializeLexer(Lexer& lexer)
{
	for (const auto iter : BinaryOperatorDatabase::Get().GetOperators())
	{
		lexer.RegisterOperatorString(iter.second.name);
	}
	for (const auto iter : UnaryOperatorDatabase::Get().GetOperators())
	{
		lexer.RegisterOperatorString(iter.second.name);
	}
}

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
	InitializeLexer(lexer);

	std::unique_ptr<ModuleAST> module_ast;
	{
		fprintf(stderr, "Potato is ready! File: %s\n", argv[1]);
		lexer.Start(argv[1]);
		Parser parser(lexer);
		module_ast = parser.ParseModule();
		lexer.End();
	}

	if (module_ast)
	{
		{
			Logger logger;
			module_ast->log(logger, "root");
		}

		CodeGen(module_ast.get());
	}

	WaitForUserToExit();
}