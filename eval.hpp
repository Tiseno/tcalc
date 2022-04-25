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

enum VType {
	VNumber,
	VFn,
	VError,
};

struct Val;

typedef N (*Fn)(N);

struct Val {
	VType type;
	N n;
	Fn fn;
};

Val makeN(N n) {
	return { VNumber, n, nullptr };
}

Val makeFn(Fn fn) {
	return { VFn, 1, fn };
}

Val makeError() {
	return { VError, 1, nullptr };
}

unordered_map<S, Val> symbols = {
	{"cos", makeFn(cos)},
	{"sin", makeFn(sin)},
	{"tan", makeFn(tan)},
	{"acos", makeFn(acos)},
	{"asin", makeFn(asin)},
	{"atan", makeFn(atan)},

	{"cosh", makeFn(cosh)},
	{"sinh", makeFn(sinh)},
	{"tanh", makeFn(tanh)},
	{"acosh", makeFn(acosh)},
	{"asinh", makeFn(asinh)},
	{"atanh", makeFn(atanh)},

	{"sqrt", makeFn(sqrt)},
	{"cbrt", makeFn(cbrt)},

	{"exp", makeFn(exp)},
	{"exp2", makeFn(exp2)},
	{"log", makeFn(log)},
	{"log10", makeFn(log10)},

	{"tgamma", makeFn(tgamma)},
	{"lgamma", makeFn(lgamma)},

	{"abs", makeFn(abs)},
	{"ceil", makeFn(ceil)},
	{"floor", makeFn(floor)},
	{"trunc", makeFn(trunc)},
	{"round", makeFn(round)},
};

unordered_map<S, Val> constants = {
	{"PI", makeN(3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628)},
	{"TAU", makeN(6.28318530717958647692528676655900576839433879875021164194988918461563281257241799725)},
	{"E", makeN(2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713)},
	{"PHI", makeN(1.61803398874989484820458683436563811772030917980576286213544862270526046281890244970)},
	{"C", makeN(299792458)},
	{"USD_SEK", makeN(9.416196)},
	{"SEK_USD", makeN(0.1062)},
	{"INF", makeN(1/(N)0)},
	{"CPW", makeN(21/(N)340)},
	{"WPC", makeN(340/(N)21)},
};

Val run(AST* ast) {
	switch(ast->t) {
		case EApply:
			{
				Val a = run(ast->e1);
				Val b = run(ast->e2);

				if(a.type == VError || b.type == VError) {
					return makeError();
				} else if(ast->op == "") {
					if(b.type != VNumber) {
						run_err("Right hand side of application must be a number");
						return makeError();
					}
					if(a.type == VNumber) {
						return makeN(a.n * b.n);
					} else if(a.type == VFn) {
						return makeN(a.fn(b.n));
					} else {
						return makeError();
					}
				} else if(ast->op == "*") {
					return makeN(a.n * b.n);
				} else if(ast->op == "/") {
					return makeN(a.n / b.n);
				} else if(ast->op == "+") {
					return makeN(a.n + b.n);
				} else if(ast->op == "-") {
					return makeN(a.n - b.n);
				} else if(ast->op == "%") {
					run_warn("Performing floating point modulo");
					auto r = a.n / b.n;
					return makeN((r - floor(r)) * b.n);
				} else if(ast->op == "^") {
					return makeN(pow(a.n, b.n));
				}
				run_err("Unrecognized operator '" + ast->op+ "'");
				return makeError();
			}
		case EValue:
			{
				return makeN(ast->n);
			}
		case ESymbol:
			{
				auto symbol = symbols.find(ast->ref);
				if(symbol == symbols.end()) {
					run_err("Could not find symbol '" + ast->ref + "'");
					return makeError();
				} else {
					return symbol->second;
				}
			}
		case EConstant:
			{
				auto constant = constants.find(ast->ref);
				if(constant == constants.end()) {
					run_err("Could not find constant '" + ast->ref + "'");
					return makeError();
				} else {
					return constant->second;
				}
			}
		case ERef:
			{
				auto n = repl_n.find(stoll(ast->ref));
				if(n == repl_n.end()) {
					run_err("Line " + ast->ref + " not executed yet");
					return makeError();
				} else {
					return makeN(n->second);
				}
			}
		case EError:
			{
				run_err("Encountered error node" + show_ast(ast));
				return makeError();
			}
		default:
			run_err("Node '" + show_ast(ast) + "' not implemented yet");
			return makeError();
	}
}

std::string show_val(Val val) {
	switch(val.type) {
		case VNumber:
			return string(ANSI_FG_YELLOW) + to_trimmed_string(val.n) + ANSI_RESET;
		case VFn:
			return string(ANSI_FG_GRAY) + "<function>" + ANSI_RESET;
		case VError:
		default:
			return string(ANSI_FG_RED) + "Error" + ANSI_RESET;
	}
}

