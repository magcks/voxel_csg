%{
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include "util.h"
#include "parser.h"

extern "C" int yylex();
extern "C" int yyparse();
extern FILE *yyin;
extern char *yytext;
 
void yyerror(const char *s);

Expr *root;
%}

// Custom types (Arg, Args, Fn, Expr) are defined in parser.h
%union {
	int  ival;
	char *sval;
	Arg  *argval;
	Args *argsval;
	Fn   *fnval;
	Expr *exprval;
}

// Task 1 - Define your tokens here
%token LEFT RIGHT CONJ DISJ NEG COMMA
%token <ival>    INT
%token <sval>    IDENT

// Task 2 - define your non-terminal types here
%type  <exprval> expr
%type  <fnval>   fn
%type  <argsval> args
%type  <argval>  arg
%%

parser:
	expr { root = $1; }
	;

// Task 2 - define production rules here
expr:
	LEFT expr RIGHT { $$ = $2; }
	| expr CONJ expr { $$ = new Expr(OP_CONJ, $1, $3); }
	| expr DISJ expr { $$ = new Expr(OP_DISJ, $1, $3); }
	| NEG expr { $$ = new Expr(OP_NEG, $2); }
	| fn { $$ = new Expr(OP_FN, $1); }
	;

fn:
	IDENT LEFT args RIGHT { $$ = new Fn($1, $3); }
	;

args:
	args COMMA arg { $1->add($3); }
	| arg { $$ = new Args; $$->add($1); }
	;

arg:
	INT { $$ = new Arg($1); }
	;
%%

#define LEXER_IMPLEMENTED

Ast parse(FILE *fp)
{
	yyin = fp;

	do {
#ifdef LEXER_IMPLEMENTED
		yyparse();
#else
		int x;
		std::cout << "Resulting tokens: ";
		while (x = yylex()) {
			std::cout << "<" << yytext << "> ";
		}
		std::cout << std::endl;
#endif

	} while (!feof(yyin));
#ifndef LEXER_IMPLEMENTED
	std::exit(EXIT_SUCCESS);
#endif

	return Ast(root);
}

void yyerror(const char *s) {
	std::cout << "Parser error: Message: " << s << std::endl;
	std::exit(EXIT_FAILURE);
}
