#pragma once

#include <iostream>

#include "ansi_code.h"
#include "types.hpp"
#include "state.hpp"
#include "lexer.hpp"

string to_trimmed_string(N n) {
	std::string r = to_string(n);
	r.erase(r.find_last_not_of('0') + 1, string::npos);
	r.erase(r.find_last_not_of('.') + 1, string::npos);
	return r;
}

void parse_debug(std::string const& s) {
	if(PARSE_DEBUG) std::cout << s << "\n";
}

enum ASTType {
	EApply,
	EValue,
	ESymbol,
	EConstant,
	ERef,
	EError,
};

struct AST {
	ASTType t;
	I i;
	N n;
	S ref;

	AST* e1;
	AST* e2;
	S op;
};

void parse_err_msg_n(I i, S const& what) {
	cout << ANSI_FG_RED << "Parse error " << i << ": " << what << ANSI_RESET << std::endl;
}

I error_count = 1;
AST* makeAstError(S const& what) {
	AST* a = new AST();
	a->t = EError;
	a->i = error_count++;
	parse_err_msg_n(a->i, what);
	return a;
}

void parse_err_msg(S const& what) {
	cout << ANSI_FG_RED << "Parse error " << error_count++ << ": " << what << ANSI_RESET << std::endl;
}

struct Parsed {
	AST* e;
	TokenIterator next;
};

// Rounding error, is it a problem?
// 2^1^8+2*1-6/9/5/5^8+2
// Now: 6
// Expected: 5.99999965867

// Left associativity of all operators except ^
// 1/2/3/4
// Now:      ((1 / 2) / 3) / 4
// Expected: ((1 / 2) / 3) / 4
// 0.0416666666667

// Left associativity of all operators except ^
// 2^3/4^5/6^7/2^3/4*3
// Now:      (((((2 ^ 3) / (4 ^ 5)) / (6 ^ 7)) / (2 ^ 3)) / 4) * 3
// Expected: (((((2 ^ 3) / (4 ^ 5)) / (6 ^ 7)) / (2 ^ 3)) / 4) * 3
// 2.61639044282e-9

// Correct precedence
// 10^6*1^2*10^6*10/10^8^1*8
// Now:      ((((10 ^ 6) * (1 ^ 2)) * (10 ^ 6)) * (10 / (10 ^ (8 ^ 1)))) * 8
// Expected: ((((10 ^ 6) * (1 ^ 2)) * (10 ^ 6)) * (10 / (10 ^ (8 ^ 1)))) * 8
// 800000

// TODO assignment
// E6    ::=    Symbol (<- E)* | (E) | Number | Const | Ref

// Grammar
// E6    ::=    (E) | Symbol | Number | Const | Ref | Prefix Prefix | Prefix Integer
// E5    ::=    E6*
// E4    ::=    E5 '^' E4 | E5
// E3    ::=    E4 ('/' E4)*
// E2    ::=    E3 ('*' E3)*
// E1    ::=    E2 (Op E2)*
// E     ::=    E1 (('+'|'-') E1)*
// P     ::=    E EOF

Parsed parse_E(const TokenIterator& current, const TokenIterator& end);

Parsed parse_E5_6(const TokenIterator& current, const TokenIterator& end) {
	parse_debug("parse_E5_6");
	bool parse_error = false;
	auto it = current;
	vector<AST*> exprs;
	while(it != end
			&& (it->type == TLParen
				|| it->type == TSymbol
				|| it->type == TNumber
				|| it->type == TConstant
				|| it->type == TPrefix)) {
		parse_debug(it->show());
		if(it->type == TLParen) {
			it = next(it);
			Parsed e = parse_E(it, end);
			it = e.next;
			if(it->type != TRParen) {
				parse_err_msg("Expected right parens"); // TODO Handle error better
				parse_error = true;
			} else {
				it = next(it);
			}
			exprs.push_back(e.e);
		} else if(it->type == TSymbol) {
			AST* a = new AST();
			a->t = ESymbol;
			a->ref = it->s;
			exprs.push_back(a);
			it = next(it);
		} else {
			if(it->type == TNumber) {
				AST* a = new AST();
				a->t = EValue;
				a->n = it->n;
				exprs.push_back(a);
			} else if (it->type == TConstant) {
				AST* a = new AST();
				a->t = EConstant;
				a->ref = it->s;
				exprs.push_back(a);
			} else if(it->type == TPrefix) {
				if(it->s == "!") {
					AST* a = new AST();
					a->op = it->s;
					a->t = ERef;
					it = next(it);
					if(it->type == TIntegral) {
						a->ref = to_string(it->i);
						exprs.push_back(a);
					} else if(it->type == TPrefix && it->s == a->op) {
						a->ref = to_string(repl_n.size() == 0 ? 0 : repl_n.size() - 1);
						exprs.push_back(a);
					} else {
						exprs.push_back(makeAstError("Expected integer or ! after !"));
					}
				} else if(it->s == "#") {
					auto prefix = it->s;
					it = next(it);
					I ref;
					if(it->type == TIntegral) {
						ref = it->i;
					} else if(it->type == TPrefix && it->s == prefix) {
						ref = repl_e.size() - 1;
					} else {
						parse_error = true;
					}

					if(parse_error) {
						exprs.push_back(makeAstError("Expected integer or # after #"));
					} else {
						auto e = repl_e.find(ref);
						if(!parse_error && e != repl_e.end()) {
							exprs.push_back(e->second);
						} else {
							exprs.push_back(makeAstError("There is no previous line"));
						}
					}
				} else {
					exprs.push_back(makeAstError("Prefix " + string(it->s) + " not implemented"));
				}
			}
			it = next(it);
		}
	}

	if (exprs.size() == 0) {
		AST* a = makeAstError("Unexpected " + it->show() + " when parsing expression");
		return { a, it };
	} else if (exprs.size() == 1) {
		return { exprs[0], it };
	} else {
		AST* a = new AST;
		a->t = EApply;
		a->e1 = exprs[0];
		a->e2 = exprs[1];
		a->op = "";

		for(size_t i = 2; i < exprs.size(); i++) {
			AST* b = new AST;
			b->t = EApply;
			b->e1 = a;
			b->e2 = exprs[i];
			b->op = "";
			a = b;
		}

		return { a, it };
	}
}

Parsed parse_E4(const TokenIterator& current, const TokenIterator& end) {
	parse_debug("parse_E4");
	Parsed e = parse_E5_6(current, end);
	auto it = e.next;
	// This does not use while as ^ is right associative
	if(it->type == T4Operator) {
		Parsed e2 = parse_E4(next(it), end);
		AST* b = new AST;
		b->t = EApply;
		b->e1 = e.e;
		b->e2 = e2.e;
		b->op = it->s;
		return { b, e2.next };
	}
	return e;
}

Parsed parse_E3(const TokenIterator& current, const TokenIterator& end) {
	parse_debug("parse_E3");
	Parsed e = parse_E4(current, end);
	auto it = e.next;
	while(it->type == T3Operator) {
		Parsed e2 = parse_E4(next(it), end);
		AST* b = new AST;
		b->t = EApply;
		b->e1 = e.e;
		b->e2 = e2.e;
		b->op = it->s;
		it = e2.next;
		e = { b, it };
	}
	return e;
}

Parsed parse_E2(const TokenIterator& current, const TokenIterator& end) {
	parse_debug("parse_E2");
	Parsed e = parse_E3(current, end);
	auto it = e.next;
	while(it->type == T2Operator) {
		Parsed e2 = parse_E3(next(it), end);
		AST* b = new AST;
		b->t = EApply;
		b->e1 = e.e;
		b->e2 = e2.e;
		b->op = it->s;
		it = e2.next;
		e = { b, it };
	}
	return e;
}

Parsed parse_E1(const TokenIterator& current, const TokenIterator& end) {
	parse_debug("parse_E1");
	Parsed e = parse_E2(current, end);
	auto it = e.next;
	while(it->type == T1Operator) {
		Parsed e2 = parse_E2(next(it), end);
		AST* b = new AST;
		b->t = EApply;
		b->e1 = e.e;
		b->e2 = e2.e;
		b->op = it->s;
		it = e2.next;
		e = { b, it };
	}
	return e;
}

Parsed parse_E(const TokenIterator& current, const TokenIterator& end) {
	parse_debug("parse_E");
	Parsed e = parse_E1(current, end);
	auto it = e.next;
	while(it->type == T0Operator) {
		parse_debug(it->show());
		Parsed e2 = parse_E1(next(it), end);
		AST* b = new AST;
		b->t = EApply;
		b->e1 = e.e;
		b->e2 = e2.e;
		b->op = it->s;
		it = e2.next;
		e = { b, it };
	}
	return e;
}

Parsed parse(const TokenIterator& current, const TokenIterator& end) {
	parse_debug("parse");
	Parsed e = parse_E(current, end);
	if(e.next->type == TEnd) {
		return e;
	}
	parse_err_msg("Did not reach end of input when parsing expression");
	return e;
}

S show_ast(AST* ast);

S show_ast(AST* ast) {
	switch(ast->t) {
		case EApply:
			if (ast->op == "") {
				return "Apply(" + show_ast(ast->e1) + ", " + show_ast(ast->e2) + ")";
			} else {
				return "Apply(" + show_ast(ast->e1)+ " " + ast->op + " " + show_ast(ast->e2) + ")";
			}
		case EValue:
			return ANSI_FG_YELLOW + to_string(ast->n) + ANSI_RESET;
		case ESymbol:
			return ANSI_FG_CYAN + ast->ref + ANSI_RESET;
		case EConstant:
			return ANSI_FG_PINK + ast->ref + ANSI_RESET;
		case ERef:
			return ANSI_FG_ORANGE + ast->op + (ast->ref == "" ? ast->op : ast->ref) + ANSI_RESET;
		case EError:
			return ANSI_FG_RED + string("Error(") + to_string(ast->i) + ")" + ANSI_RESET;
	}
	return ANSI_FG_GRAY + string("Unknown") + ANSI_RESET;
}

void print_ast(AST* ast) {
	if(ast->t == EError)
		return;
	cout << show_ast(ast) << endl;
}

S pretty_show_ast(AST* ast, size_t level) {
	switch(ast->t) {
		case EApply: {
			if (ast->op == "") {
				return pretty_show_ast(ast->e1, level+1) + " " + pretty_show_ast(ast->e2, level+1);
			} else {
				S s = "";
				if(level != 0) {
					s += "(";
				}
				s += pretty_show_ast(ast->e1, level+1) + "" + ast->op + "" + pretty_show_ast(ast->e2, level+1);
				if(level != 0) {
					s += ")";
				}
				return s;
			}
		}
		case EValue: {
			return ANSI_FG_YELLOW + to_trimmed_string(ast->n) + ANSI_RESET;
		}
		case ESymbol:
			return ANSI_FG_CYAN + ast->ref + ANSI_RESET;
		case EConstant:
			return ANSI_FG_PINK + ast->ref + ANSI_RESET;
		case ERef:
			return ANSI_FG_ORANGE + ast->op + (ast->ref == "" ? ast->op : ast->ref) + ANSI_RESET;
		case EError:
			return ANSI_FG_RED + string("Error(") + to_string(ast->i) + ")" + ANSI_RESET;
	}
	return ANSI_FG_GRAY + string("Unknown") + ANSI_RESET;
}

void pretty_print_ast(AST* ast) {
	cout << pretty_show_ast(ast, 0) << endl;
}

bool ast_has_errors(AST* ast) {
	switch(ast->t) {
		case EApply:
			return ast_has_errors(ast->e1) || ast_has_errors(ast->e2);
		case EError:
			return true;
		default:
			return false;
	}
}

