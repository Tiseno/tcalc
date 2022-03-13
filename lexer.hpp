#pragma once

#include <vector>
#include <bitset>
#include <iostream>

#include "types.hpp"
#include "state.hpp"

enum TokenType {
	TSymbol,
	TIntegral,
	TNumber,
	TConstant,
	TLParen,
	TRParen,
	T0Operator,
	T1Operator,
	T2Operator,
	T3Operator,
	T4Operator,
	T5Operator,
	TPrefix,
	TEnd,
};

struct Token {
	TokenType type;
	S s;
	I i;
	N n;
	S err;

	S show_err() const {
		if(err == "")
			return "";
		return "!!" + err + "!!";
	}

	S show() const {
		if(type == TSymbol) {
			return "Symbol[" + s + "]" + show_err();
		} else if(type == TIntegral) {
			return "Integral[" + to_string(i) + "]" + show_err();
		} else if(type == TNumber) {
			return "Number[" + to_string(n) + "]" + show_err();
		} else if(type == TConstant) {
			return "Constant[" + s + "]" + show_err();
		} else if(type == T0Operator) {
			return "Operator0[" + s + "]" + show_err();
		} else if(type == T1Operator) {
			return "Operator1[" + s + "]" + show_err();
		} else if(type == T2Operator) {
			return "Operator2[" + s + "]" + show_err();
		} else if(type == T3Operator) {
			return "Operator3[" + s + "]" + show_err();
		} else if(type == T4Operator) {
			return "Operator4[" + s + "]" + show_err();
		} else if(type == T5Operator) {
			return "Operator5[" + s + "]" + show_err();
		} else if(type == TPrefix) {
			return "Prefix[" + s + "]" + show_err();
		} else if(type == TLParen) {
			return "LParen" + show_err();
		} else if(type == TRParen) {
			return "RParen" + show_err();
		} else if(type == TEnd) {
			return "End" + show_err();
		}
		return "TokenNotImplemented[" + s + "]" + show_err();
	}
};

typedef vector<Token> Tokens;

typedef Tokens::iterator TokenIterator;

bool isCapitalLetter(C c) {
	return (c >= 65 && c <= 90);
}

bool isAsciiPrefix(C c) {
	return c == '!' || c == '@' || c == '#';
}

bool isPrefix(S s) {
	return s == "!" || s == "@" || s == "#" || s == "¤" || s == "§" ;
}

bool isAsciiOperator(C c) {
	return c != '.' && !isspace(c) && !isPrefix(string(1, c))
		&& ((c >= 0 && c <= 47)
				|| (c >= 58 && c <= 64)
				|| (c >= 91 && c <= 94)
				|| (c >= 123));
}

void skip_space(F f) {
	C c = f->peek();
	while(f->good() && isspace(c)) {
		f->get(c);
		c = f->peek();
	}
}

Token lex_integral(F f) {
	C c = f->peek();
	S s = "0";
	while(isdigit(c)) {
		f->get(c);
		s += c;
		c = f->peek();
	}
	I i = 1;
	S err = "";
	try {
		i = stoll(s);
	} catch(...) {
		err = "Exception when parsing integral '" + s + "'";
	}

	return { TIntegral, s, i, 1, err };
}

Token lex_number(F f) {
	C c = f->peek();
	S s = "0";
	while(isdigit(c)) {
		f->get(c);
		s += c;
		c = f->peek();
	}
	if(c == '.') {
		f->get(c);
		s += c;
		c = f->peek();
	}
	while(isdigit(c)) {
		f->get(c);
		s += c;
		c = f->peek();
	}
	N n = 1;
	S err = "";
	try {
		n = stold(s);
	} catch(...) {
		err = "Exception when parsing number '" + s + "'";
	}

	return { TNumber, s, 1, n, err };
}

Token lex_constant(F f) {
	C c = f->peek();
	S s = "";
	while(isCapitalLetter(c) || c == '_') {
		f->get(c);
		s += c;
		c = f->peek();
	}
	return { TConstant, s, 1, 1, "" };
}

Token lex_symbol(F f) {
	C c = f->peek();
	S s = "";
	S prefix = "";
	// Supports UTF-8, and we need to do it explicitly so we can break on our supported symbols
	while(c == '_' || isPrefix(string(1, c)) || (!isdigit(c) && !isspace(c) && !isAsciiOperator(c) && !isCapitalLetter(c) && c != EOF)) {
		I begin = f->tellg();
		if(c >= 0) { // ASCII char
			if(isPrefix(string(1, c))) {
				if(s.size() == 0) {
					prefix = string(1, c);
				} else {
					f->seekg(begin);
					f->peek();
					break;
				}
			}
			f->get(c);
			s += c;
			c = f->peek();
			continue;
		}
		// UTF-8 first code point
		bitset<8> b(c);
		N num_bytes = 0;
		for(int j = 7; j >= 0; j--) {
			bitset<8> mask = 1;
			auto shifted = (b >> j);
			auto bit = shifted & mask;
			if(bit[0]) {
				num_bytes++;
			} else {
				break;
			}
		}
		S u8c = "";
		for(int i = 0; i < num_bytes; i++) {
			f->get(c);
			u8c += c;
		}

		if(isPrefix(u8c)) {
			if(s.size() == 0) {
				prefix = u8c;
			} else {
				f->seekg(begin);
				f->peek();
				break;
			}
		}

		s += u8c;
		c = f->peek();
	}

	// TODO check prefix and make reference token
	// if(prefix == "")
	// 	;
	// else if(prefix == "§")
	// 	cout << "REFERENCE" << endl;
	// else
	// 	cout << "UNKNOWN PREFIX" << prefix << endl;
	return { TSymbol, s, 1, 1, "" };
}

Tokens lex_line(F f) {
	skip_space(f);
	Tokens tokens;
	C c = f->peek();
	while(f->good()){
		if(isCapitalLetter(c)) {
			tokens.push_back(lex_constant(f));
		} else if(isAsciiPrefix(c)) {
			C prefix = c;
			f->get(c);
			tokens.push_back({ TPrefix, string(1, prefix), 1, 1, "" });
			c = f->peek();
			// Reference as value and reference as expression
			if (isdigit(c)) {
				tokens.push_back(lex_integral(f));
			}
		} else if(c == '.' || isdigit(c)) {
			tokens.push_back(lex_number(f));
		} else if(c == '(') {
			f->get(c);
			tokens.push_back({ TLParen, "", 1, 1, "" });
		} else if(c == ')') {
			f->get(c);
			tokens.push_back({ TRParen, "", 1, 1, "" });
		} else if(isAsciiOperator(c)) {
			S s = "";
			s += c;
			f->get(c);
			C c2 = f->peek();
			if(isAsciiOperator(c2)) {
				f->get(c2);
				s += c2;
			}
			if(c == '+' || c == '-') {
				tokens.push_back({ T0Operator, s, 1, 1, "" });
			} else if(c == '*') {
				tokens.push_back({ T2Operator, s, 1, 1, "" });
			} else if(c == '/') {
				tokens.push_back({ T3Operator, s, 1, 1, "" });
			} else if(c == '^') {
				tokens.push_back({ T4Operator, s, 1, 1, "" });
			} else {
				tokens.push_back({ T1Operator, s, 1, 1, "" }); // This is all custom operators precedence
			}
		} else {
			tokens.push_back(lex_symbol(f));
		}
		skip_space(f);
		c = f->peek();
	}
	tokens.push_back({ TEnd, "", 1, 1, "" });
	return tokens;
}

void print_tokens(const Tokens& tokens) {
	for(const auto& s : tokens) {
		cout << s.show() << " ";
	}
	if(tokens.size() > 0) {
		cout << endl;
	}
}

