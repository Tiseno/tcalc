#pragma once

#include "types.hpp"
#include "state.hpp"
#include "lexer.hpp"

void err(S what) {
	cout << "ERROR: " << what << std::endl;
	error = true;
	if(!REPL) throw 0; // TODO handle error better
}

enum ASTType {
	EApply,
	EValue,
	ERef,
};

struct AST;

struct AST {
	ASTType t;
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

// Grammar
// E2    ::=    E2* | (E) | Symbol | Number | Const
// E1    ::=    E2 Op E1 | E2
// E     ::=    E1 + E | E1 - E | E1 | EOF

Parsed parse_E(const TokenIterator& current, const TokenIterator& end);

Parsed parse_E2(const TokenIterator& current, const TokenIterator& end) {
	auto it = current;
	vector<AST*> exprs;
	while(it != end
			&& (it->type == TLParen
				|| it->type == TSymbol
				|| it->type == TNumber
				|| it->type == TConstant)) {
		if(it->type == TLParen) {
			it = next(it);
			Parsed e = parse_E(it, end);
			it = e.next;
			if(it->type != TRParen) {
				err("Expected right parens");
                // TODO Handle error better
			}
			it = next(it);
			exprs.push_back(e.e);
		} else {
			if(it->type == TNumber || it->type == TConstant) {
				AST* a = new AST();
				a->t = EValue;
				a->n = it->n;
				exprs.push_back(a);
			} else {
				// TODO SYMBOLS TO REF
				AST* a = new AST();
				a->t = EValue;
				a->n = it->n;
				exprs.push_back(a);
			}
			it = next(it);
		}
	}

	if (exprs.size() == 0) {
		err("No E2 expression");
        // TODO Handle error better
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

Parsed parse_E1(const TokenIterator& current, const TokenIterator& end) {
	Parsed e = parse_E2(current, end);
	auto it = e.next;
	if(it->type == T2Operator) {
		Parsed e2 = parse_E1(next(it), end);
		AST* b = new AST;
		b->t = EApply;
		b->e1 = e.e;
		b->e2 = e2.e;
		b->op = it->s;
		return { b, e2.next };
	}
	return e;
}

Parsed parse_E(const TokenIterator& current, const TokenIterator& end) {
	// if(current-current->type == TEOF) {
	// 	return { EEOF, end };
	// }
	Parsed e = parse_E1(current, end);
	auto it = e.next;
	if(it->type == T1Operator) {
		Parsed e2 = parse_E(next(it), end);
		AST* b = new AST;
		b->t = EApply;
		b->e1 = e.e;
		b->e2 = e2.e;
		b->op = it->s;
		return { b, e2.next };
	}
	return e;
}

void print_astr(AST* ast);

void print_astr(AST* ast) {
	switch(ast->t) {
		case EApply:
			cout << "(";
			print_astr(ast->e1);
			cout << " " << ast->op << " ";
			print_astr(ast->e2);
			cout << ")";
			return;
		case EValue:
			cout << ast->n;
			return;
		case ERef:
			cout << "REF#" << ast->ref;
			return;
	}
}

void print_ast(AST* ast) {
	print_astr(ast);
	cout << endl;
}

