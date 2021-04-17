#include <unordered_map>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <cassert>
#include <climits>
#include <iomanip>
using namespace std;

bool REPL = true;

bool error = false;
bool debug = false;

const char ref_prefix = '!';
const char ref_s_prefix = '?';

typedef unsigned long long UN;
typedef long long N;


unordered_map<UN, N> repl_n;
unordered_map<UN, string> repl_s;


static void pl(string s) {
	cout << s << endl;
}

static void p(string s) {
	cout << s;
}

static void debugp(string s) {
	if(debug) {
		p(s);
	}
}

typedef istream* F;

void skip_to(F f, char end) {
	char c = f->peek();
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

void skip_white(F f) {
	char c = f->peek();
	while(f->good() && (isspace(c) || c == '#')) {
		if(c == '#') { while(c != '\n') { f->get(c); c = f->peek(); }}
		f->get(c);
		c = f->peek();
	}
}

bool issymbolstart(char c) {
	return isalpha(c);
}

void err(string const& what) {
	std::cout << "ERROR: " << what << std::endl;
	error = true;
	if(!REPL) throw 0;
}


N parse_N(F f) {
	debugp("[parse_N]");
	char c = f->peek();
	string s = "";
	while(isdigit(c)) {
		f->get(c);
		s += c;
		c = f->peek();
	}
	N n;
	try {
		n = stoll(s);
	} catch(...) {
		cout << flush;
		err("Parsing n (" + s + ")");
		n = 1;
	}
	return n;
}

N parse_UN(F f) {
	debugp("[parse_UN]");
	char c = f->peek();
	string s = "";
	while(isdigit(c)) {
		f->get(c);
		s += c;
		c = f->peek();
	}
	N n;
	try {
		return stoull(s);
	} catch(...) {
		cout << flush;
		err("Parsing UN (" + s + ")");
		return 1;
	}
}

N get_REF(F f) {
	char c;
	f->get(c);
	assert(c == ref_prefix);
	N n = parse_UN(f);
	if (repl_n.find(n) == repl_n.end()) {
		err("Ref not found (" + to_string(n) + ")");
		return 1;
	}
	return repl_n[n];
}

N get_s_REF(F f) {
	char c;
	f->get(c);
	assert(c == ref_s_prefix);
	N n = parse_UN(f);
	if (repl_n.find(n) == repl_n.end()) {
		err("Ref not found (" + to_string(n) + ")");
		return 1;
	}

	cout << repl_s[n] << endl;
	return 1;
}

N parse_line(F f) {
	skip_white(f);
	char c = f->peek();
	if(isdigit(c)) {
		return parse_N(f);
	}
	if(c == ref_prefix) {
		return get_REF(f);
	}
	if(c == ref_s_prefix) {
		return get_s_REF(f);
	}
	err("Parsing line.");
	return 1;
}

int main(int argc, char *argv[]) {
	REPL = true;
	UN repl_counter = 0;
	while(true) {
		string in;
		cout << repl_counter << " <- ";
		getline(cin,in);
		if(in.length() == 0)
			continue;

		stringstream str_stream(in);
		N n = parse_line(&str_stream);
		if(!error) {
			repl_n[repl_counter] = n;
			repl_s[repl_counter] = in;
			repl_counter++;
			cout << n << endl;
		}
		skip_on_error(&str_stream);
		error = false;
	}
	return 0;
}
