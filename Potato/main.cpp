#include "stdafx.h"

#include "lexer.h"
#include "parser.h"
#include "operator_database.h"
#include "utils.h"
#include "ast_structure.h"

bool CodeGen(ModuleAST* module_root, const std::string& file_name);

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
		Utils::Log("Press return to exit..");
		getchar();
	};

	if (argc < 2)
	{
		Utils::LogError("No file specified!\n");
		WaitForUserToExit();
		return EXIT_FAILURE;
	}

	Utils::Log("Potato is ready! File:", argv[1]);

	std::unique_ptr<ModuleAST> module_ast;
	{
		Lexer lexer;
		InitializeLexer(lexer);
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
		Utils::LogSuccess("PARSED SUCCESSFULLY!");
		Utils::Log("Codegen:");
		CodeGen(module_ast.get(), argv[1]);
	}

	WaitForUserToExit();
}