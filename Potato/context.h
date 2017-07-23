#pragma once

class Context
{
public:
	Context();

	llvm::LLVMContext the_context;
	llvm::IRBuilder<> builder;
	std::unique_ptr<llvm::Module> module;

	//class data

	//function data

	//scope data
	// std::map<std::string, llvm::AllocaInst *> NamedValues;
};
