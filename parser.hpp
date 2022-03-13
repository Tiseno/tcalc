#pragma once

#include <iostream>

#include "types.hpp"
#include "state.hpp"
#include "lexer.hpp"

void parse_debug(std::string const& s) {
	if(PARSE_DEBUG) std::cout << s << "\n";
}

void parse_warn(S const& what) {
	cout << "Warning: " << what << std::endl;
}

void parse_err(S const& what) {
	cout << "Error: " << what << std::endl;
	parse_error = true;
}

enum ASTType {
	EApply,
	EValue,
	ESymbol,
	EError,
};

struct AST;

struct AST {
	ASTType t;
	I i;
	N n;
	S ref;

	AST* e1;
	AST* e2;
	S op;
};

struct Parsed {
	AST* e;
	TokenIterator next;
};

// Left associativity of all operators except ^
// 1/2/3/4
// Now: (1 / (2 / (3 / 4)))
// Expected: ((1 / 2) / 3) / 4
// 0.0416666666667
// 2^3/4^5/6^7/2^3/4*3
// Now:      (2 ^ (3 / (4 ^ (5 / (6 ^ (7 / (2 ^ (3 / (4 * 3)))))))))
// Expected: (((((2 ^ 3) / (4 ^ 5)) / (6 ^ 7)) / (2 ^ 3)) / 4) * 3
// 2.61639044282e-9

// Correct precedence
// 10^6*1^2*10^6*10/10^8^1*8
// Now:      (10 ^ (6 * (1 ^ (2 * (10 ^ (6 * (10 / (10 ^ (8 ^ (1 * 8))))))))))
// Expected: (((((10 ^ 6) * (1 ^ 2)) * (10 ^ 6)) * 10) / (10 ^ (8 ^ 1))) * 8
// 800000

// TODO assignment
// E6    ::=    Symbol (<- E)* | (E) | Number | Const | Ref

// Grammar
// E6    ::=    (E) | Symbol | Number | Const | Ref
// E5    ::=    E6*
// E4    ::=    E5 '^' E4 | E4
// E3    ::=    E4 ('/' E4)*
// E2    ::=    E3 ('*' E3)*
// E1    ::=    E2 (Op E2)*
// E     ::=    E1 (('+'|'-') E1)*
// P     ::=    E EOF

Parsed parse_E(const TokenIterator& current, const TokenIterator& end);

Parsed parse_E5_6(const TokenIterator& current, const TokenIterator& end) {
	parse_debug("parse_E5_6");
	auto it = current;
	vector<AST*> exprs;
	while(it != end
			&& (it->type == TLParen
				|| it->type == TSymbol
				|| it->type == TNumber
				|| it->type == TConstant
				|| it->type == TRefIntegral
				|| it->type == TRefSymbol)) {
		parse_debug(it->show());
		if(it->type == TLParen) {
			it = next(it);
			Parsed e = parse_E(it, end);
			it = e.next;
			if(it->type != TRParen) {
				parse_err("Expected right parens"); // TODO Handle error better
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
				a->t = EValue;
				auto constant = constants.find(it->s);
				if(constant == constants.end()) {
					parse_err("Could not find constant '" + it->s + "'");
					a->t = EError;
				} else {
					a->n = constant->second;
				}
				exprs.push_back(a);
			} else if(it->type == TRefIntegral ) {
				AST* a = new AST();
				a->t = EValue;
				auto n = repl_n.find(it->i);
				if(n == repl_n.end()) {
					parse_err("Line " + to_string(it->i) + " not executed yet");
					a->t = EError;
				} else {
					a->n = n->second;
				}
				exprs.push_back(a);
			} else if(it->type == TRefSymbol) {
				AST* a = new AST();
				a->t = EValue;
				auto n = refs.find(it->s);
				if(n == refs.end()) {
					parse_err("Could not find reference '" + it->s + "'");
					a->t = EError;
				} else {
					a->n = n->second;
				}
				exprs.push_back(a);
			}
			it = next(it);
		}
	}

	if (exprs.size() == 0) {
		parse_err("Unexpected " + it->show() + " when parsing E2 expression");
		AST* a = new AST;
		a->t = EError;
		return { a, it };
	}

	if (exprs.size() == 1) {
		return { exprs[0], it };
	}

	AST* a = new AST;
	a->t = EApply;
	a->e1 = exprs[0];
	a->op = "*";
	AST* b = a;
	for(size_t i = 1; i < exprs.size(); i++) {
		if(i >= exprs.size()-1) { // Last element, we insert ourselves as the second expression
			b->e2 = exprs[i];
			break;
		} else { // If not last element we insert an application for this and the next element as second expression
			b->e2 = new AST;
			b->e2->t = EApply;
			b->e2->e1 = exprs[i];
			b->e2->op = "*";
			b = b->e2;
		}
	}

	return { a, it };
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
	parse_err("Did not reach end of input when parsing expression");
	return e;
}

S show_ast(AST* ast);

S show_ast(AST* ast) {
	switch(ast->t) {
		case EApply:
			return "Apply(" + show_ast(ast->e1)+ " " + ast->op + " " + show_ast(ast->e2) + ")";
		case EValue:
			return to_string(ast->n);
		case ESymbol:
			return ast->ref;
		case EError:
			return "Error";
	}
	return "Unknown";
}

void print_ast(AST* ast) {
	if(ast->t == EError)
		return;
	cout << show_ast(ast) << endl;
}

S pretty_show_ast(AST* ast) {
	switch(ast->t) {
		case EApply:
			return "(" + pretty_show_ast(ast->e1) + " " + ast->op + " " + pretty_show_ast(ast->e2) + ")";
		case EValue: {
			std::string r = to_string(ast->n);
			r.erase(r.find_last_not_of('0') + 1, string::npos);
			r.erase(r.find_last_not_of('.') + 1, string::npos);
			return r;
	}
		case ESymbol:
			return ast->ref;
		case EError:
			return "Error";
	}
	return "Unknown";
}

void pretty_print_ast(AST* ast) {
	cout << pretty_show_ast(ast) << endl;
}

