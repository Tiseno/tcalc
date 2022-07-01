#include <iostream>
#include <algorithm>

#include <readline/readline.h>
#include <readline/history.h>

#include "ansi_code.h"
#include "types.hpp"
#include "state.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "eval.hpp"

using namespace std;

int main(int argc, char * argv[]) {
	string argument_expr = "";
	if(argc > 1) {
		vector<string> args;
		args.assign(argv + 1, argv + argc);
		for(auto as : args)
			argument_expr += as + " ";
		replace(argument_expr.begin(), argument_expr.end(), '\n', ' ');
	}

	REPL = true;

	I repl_counter = 0;
	while(true) {
		S prompt = ANSI_FG_GREEN_DARK + to_string(repl_counter) + " <- " + ANSI_RESET;
		S in;
		if(argument_expr == "") {
			in = readline(prompt.c_str());
			if(in.length() == 0) {
				cout << "\e[A" << '\r' << "                    " << endl;
				continue;
			}
			add_history(in.c_str());
		} else {
			// TODO this is disgusting
			cout << prompt << argument_expr << "\n";
			in = argument_expr;
			argument_expr = "";
		}

		stringstream str_stream(in);
		Tokens tokens = lex_line(&str_stream);
#ifdef DEBUG
		print_tokens(tokens);
#endif

		Parsed parsed = parse(tokens.begin(), tokens.end());
#ifdef DEBUG
		print_ast(parsed.e);
#endif

		if(parsed.e->t == EApply)
			pretty_print_ast(parsed.e);

		if(parse_error) {
			parse_error = false;
			continue;
		}

		Val out = run(parsed.e);

		if(out.type != VError) {
			// TODO save value not n
			repl_n[repl_counter] = out.n;
			repl_e[repl_counter] = parsed.e;
			repl_counter++;
			cout << show_val(out) << endl;
		}
		cout << endl;
	}
	return 0;
}
