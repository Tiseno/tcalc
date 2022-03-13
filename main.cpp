#include <iostream>

#include <readline/readline.h>
#include <readline/history.h>

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
		S prompt = to_string(repl_counter) + " <- ";
		S in = readline(prompt.c_str());
		if(in.length() == 0) {
			cout << "\e[A" << '\r' << "                    " << endl;
			continue;
		}
		add_history(in.c_str());

		stringstream str_stream(in);
		Tokens tokens = lex_line(&str_stream);
#ifdef DEBUG
		print_tokens(tokens);
#endif

		Parsed ast = parse(tokens.begin(), tokens.end());
#ifdef DEBUG
		print_ast(ast.e);
		pretty_print_ast(ast.e);
#endif

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
