#include <iostream>

#include "types.hpp"
#include "state.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "eval.hpp"

using namespace std;

int main() {
	REPL = true;

	I repl_counter = 0;
	while(true) {
		S in;
		cout << repl_counter << " <- ";
		getline(cin,in);
		if(in.length() == 0) {
			cout << "\e[A" << '\r' << "                    " << endl;
			continue;
		}

		stringstream str_stream(in);
		Tokens tokens = lex_line(&str_stream);
		// print_tokens(tokens);

		Parsed ast = parse_E(tokens.begin(), tokens.end());
		// print_ast(ast.e);

		N out = eval(ast.e);

		if(!error) {
			repl_n[repl_counter] = out;
			repl_s[repl_counter] = in;
			repl_counter++;
			cout << out;
		}
		error = false;
		cout << endl;
	}
	return 0;
}
