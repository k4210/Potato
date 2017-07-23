#include "stdafx.h"

#include "potato_common.h"
#include "ast_structure.h"
#include "context.h"

bool CodeGen(ModuleAST* module_root)
{
	return module_root;
}

Context::Context()
	: builder(the_context)
{}