#pragma once

#include <cmath>

#include "state.hpp"
#include "types.hpp"
#include "parser.hpp"

void run_warn(S what) {
	cout << "Warning: " << what << std::endl;
}

void run_err(S what) {
	cout << ANSI_FG_RED << "Error: " << what << ANSI_RESET << std::endl;
}

struct Val {
	N n;
	E error;
};

Val run(AST* ast) {
	switch(ast->t) {
		case EApply:
			{
				Val a = run(ast->e1);
				Val b = run(ast->e2);

				if(a.error || b.error) {
					return { 0, true };
				} else if(ast->op == "*") {
					return { a.n * b.n, false };
				} else if(ast->op == "/") {
					return { a.n / b.n, false };
				} else if(ast->op == "+") {
					return { a.n + b.n, false };
				} else if(ast->op == "-") {
					return { a.n - b.n, false };
				} else if(ast->op == "%") {
					run_warn("Performing floating point modulo");
					auto r = a.n / b.n;
					return { (r - floor(r)) * b.n, false };
				} else if(ast->op == "^") {
					return { pow(a.n, b.n), false };
				}
				run_err("Unrecognized operator '" + ast->op+ "'");
				return { 0, true };
			}
		case EValue:
			{
				return { ast->n, false };
			}
		case ESymbol:
			{
				// TODO evaluate symbols
				run_err("Could not evaluate symbol '" + ast->ref + "'");
				return { 0, true };
			}
		case ERef:
			{
				auto n = repl_n.find(stoll(ast->ref));
				if(n == repl_n.end()) {
					run_err("Line " + ast->ref + " not executed yet");
					return { 0, true };
				} else {
					return { n->second, false };
				}
			}
		case EError:
			{
				run_err("Encountered error node" + show_ast(ast));
				return { 0, true };
			}
		default:
			run_err("Node '" + show_ast(ast) + "' not implemented yet");
			return { 0, true };
	}
}

