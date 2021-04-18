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

		Parsed ast = parse(tokens.begin(), tokens.end());
		// print_ast(ast.e);

		if(parse_error) {
			parse_error = false;
			continue;
		}

		Val out = run(ast.e);

		if(!out.error) {
			repl_n[repl_counter] = out.n;
			repl_s[repl_counter] = in;
			repl_counter++;
			cout << out.n << endl;
		}
	}
	return 0;
}
