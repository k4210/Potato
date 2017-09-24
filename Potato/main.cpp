#include "stdafx.h"

#include "lexer.h"
#include "parser.h"
#include "operator_database.h"
#include "utils.h"
#include "ast_structure.h"

bool CodeGen(ModuleAST* module_root, const std::vector<std::shared_ptr<ModuleData>>& known_modules, const std::string& file_name);

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

	std::vector<std::shared_ptr<ModuleData>> compiled_modules;
	{
		std::unique_ptr<ModuleAST> module_ast;

		{
			Lexer lexer;
			InitializeLexer(lexer);
			lexer.Start(argv[1]);
			Parser parser(lexer);
			module_ast = parser.ParseModule();
		}

		if (module_ast)
		{
			Logger logger;
			module_ast->log(logger, "root");
		}

		if (module_ast)
		{
			Utils::LogSuccess("PARSED SUCCESSFULLY!");
			Utils::Log("Codegen:");
			const bool compiled_successfully = CodeGen(module_ast.get(), compiled_modules, argv[1]);
			if (compiled_successfully && module_ast->module_data_)
			{
				compiled_modules.emplace_back(module_ast->module_data_);
				Utils::LogSuccess("MODULE", module_ast->module_data_->name.c_str(), "WAS COMPILED SUCCESSFULLY!");
			}
		}
	}

	WaitForUserToExit();
}