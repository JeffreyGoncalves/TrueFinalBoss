/* attention: NEW est defini dans tp.h Utilisez un autre nom de token */

%token IS CLASS VAR EXTENDS DEF OVERRIDE IF THEN ELSE AFF OBJECT
%token ADD MIN MUL DIV
%token ';' ',' '.' ':' '{' '}' '(' ')' 
%token<S> Id IdClass
%token<I> Cste
%token<C> RelOp

%{
#include "tp.h"
#include "tp_y.h"

extern int yylex();
extern void yyerror(char *);
%}

%%
Prog : classLOpt block
;

classLOpt:
;

block:
;
