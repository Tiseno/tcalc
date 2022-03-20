#pragma once

#include <unordered_map>

#include "types.hpp"

struct AST;

unordered_map<I, N> repl_n;
unordered_map<I, AST*> repl_e;

bool REPL = true;

bool parse_error = false;

const bool PARSE_DEBUG = false;

bool pretty_print = true;

