#include <unordered_map>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <cassert>
#include <climits>
#include <iomanip>
#include <vector>
#include <bitset>
#include <cmath>
#include <cstdio>

using namespace std;

bool REPL = true;

bool error = false;

typedef char C;
typedef string S;
typedef unsigned long long I; // Used for indexing
typedef long double N; // Number type

enum TokenType {
	TSymbol,
	TNumber,
	TConstant,
	TLParen,
	TRParen,
	T1Operator,
	T2Operator,
	TEOF,
};

const unordered_map<string, N> constants = {
	{"PI", 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628},
	{"TAU", 6.28318530717958647692528676655900576839433879875021164194988918461563281257241799725},
	{"E", 2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713},
	{"PHI", 1.61803398874989484820458683436563811772030917980576286213544862270526046281890244970},
	{"USD_TO_SEK", 8.44},
	{"SEK_TO_USD", 0.118483412322},
};

struct Token {
	TokenType type;
	S s;
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
		} else if(type == TNumber) {
			return "Number[" + to_string(n) + "]" + show_err();
		} else if(type == TConstant) {
			return "Constant[" + s + "]" + show_err();
		} else if(type == T2Operator) {
			return "Operator[" + s + "]" + show_err();
		} else if(type == TLParen) {
			return "LParen" + show_err();
		} else if(type == TRParen) {
			return "RParen" + show_err();
		} else if(type == TEOF) {
			return "EOF" + show_err();
		}
		return "TokenNotImplemented[" + s + "]" + show_err();
	}
};

unordered_map<S, N> refs;

unordered_map<I, N> repl_n;
unordered_map<I, S> repl_s;

typedef istream* F;

void skip_to(F f, C end) {
	C c = f->peek();
	while(f->good() && c != end) {
		f->get(c);
		c = f->peek();
	}
}

void skip_to_lf(F f) {
	skip_to(f, '\n');
}

void skip_on_error(F f) {
	if(REPL && error) {
		skip_to_lf(f);
	}
}

void skip_space(F f) {
	char c = f->peek();
	while(f->good() && (c == ' ' || c == '\t')) {
		f->get(c);
		c = f->peek();
	}
}

void skip_white(F f) {
	C c = f->peek();
	while(f->good() && (isspace(c))) { // || c == '#')) {
		// if(c == '#') { while(c != '\n') { f->get(c); c = f->peek(); }}
		f->get(c);
		c = f->peek();
	}
}

bool issymbolstart(C c) {
	return isalpha(c);
}

void err(S what) {
	cout << "ERROR: " << what << std::endl;
	error = true;
	if(!REPL) throw 0;
}

bool isCapitalLetter(C c) {
	return (c >= 65 && c <= 90);
}

bool isPrefix(S s) {
	return s == "§" || s == "@" || s == "#" || s == "¤";
}

bool isAsciiOperator(C c) {
	return !isPrefix(string(1, c))
		&& ((c >= 0 && c <= 47)
				|| (c >= 58 && c <= 64)
				|| (c >= 91 && c <= 94)
				|| (c >= 123));
}

bool isAsciiSymbolOrDecimal(C c) {
	return isAsciiOperator(c) || isdigit(c);
}

Token lex_number(F f) {
	C c = f->peek();
	S s = "";
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

	return { TNumber, s, n, err };
}

Token lex_constant(F f) {
	C c = f->peek();
	S s = "";
	while(isCapitalLetter(c) || c == '_') {
		f->get(c);
		s += c;
		c = f->peek();
	}
	auto n = constants.find(s);
	if(n == constants.end()) {
		return { TConstant, s, 1, "Could not find constant '" + s + "'" };
	}
	return { TConstant, s, n->second, "" };
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
	return { TSymbol, s, 0, "" };
}

vector<Token> lex_line(F f) {
	skip_white(f);
	vector<Token> tokens;
	C c = f->peek();
	while(f->good()){
		if(isCapitalLetter(c)) {
			tokens.push_back(lex_constant(f));
		} else if(isdigit(c)) {
			tokens.push_back(lex_number(f));
		} else if(c == '(') {
			f->get(c);
			tokens.push_back({ TLParen, "", 0, "" });
		} else if(c == ')') {
			f->get(c);
			tokens.push_back({ TRParen, "", 0, "" });
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
				tokens.push_back({ T1Operator, s, 0, "" });
			} else {
				tokens.push_back({ T2Operator, s, 0, "" });
			}
		} else {
			tokens.push_back(lex_symbol(f));
		}
		skip_space(f);
		c = f->peek();
	}
	tokens.push_back({ TEOF, "", 0, "" });
	return tokens;
}

void print_tokens(const vector<Token>& tokens) {
	for(const auto& s : tokens) {
		cout << s.show() << " ";
	}
	cout << endl;
}

typedef std::vector<Token>::iterator TokenIterator;

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
				throw 1;
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
		throw 1;
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

// E2    ::=    E2* | (E) | Symbol | Number | Const
// E1    ::=    E2 Op E1 | E2
// E     ::=    E1 + E | E1 - E | E1 | EOF
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

// E2    ::=    E2* | (E) | Symbol | Number | Const
// E1    ::=    E2 Op E1 | E2
// E     ::=    E1 + E | E1 - E | E1 | EOF
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

int main() {
	REPL = true;
	I repl_counter = 0;
	while(true) {
		S in;
		cout << repl_counter << " <- ";
		getline(cin,in);
		if(in.length() == 0)
			continue;

		stringstream str_stream(in);
		vector<Token> tokens = lex_line(&str_stream);
		Parsed ast = parse_E(tokens.begin(), tokens.end());
		// print_tokens(tokens);
		// print_ast(ast.e);
		N out = eval(ast.e);
		if(!error) {
			repl_n[repl_counter] = out;
			repl_s[repl_counter] = in;
			repl_counter++;
			cout << out;
		}
		skip_on_error(&str_stream);
		error = false;
		cout << endl;
	}
	return 0;
}
