#pragma once

#include "types.hpp"
#include "parser.hpp"

N eval(AST* ast) {
	switch(ast->t) {
		case EApply:
			{
				N a = eval(ast->e1);
				N b = eval(ast->e2);
				if(ast->op == "*") {
					return a * b;
				} else if(ast->op == "/") {
					return a / b;
				} else if(ast->op == "+") {
					return a + b;
				} else if(ast->op == "-") {
					return a - b;
				}
				return a;
			}
		case EValue:
			{
				return ast->n;
			}
		case ERef:
			{
				// TODO look up ref
				return 1;
			}
		default:
			return 1;
	}
}

