#pragma once

#include <memory>
#include <vector>

class HighLevelAST;
class Lexer;

class Parser
{
public:
	std::vector<std::unique_ptr<HighLevelAST>> ParseModule(Lexer& lexer);
};