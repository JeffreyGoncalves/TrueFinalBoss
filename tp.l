Chiffre [0-9]
Entier {Chiffre}+
Lower	[a-z]
Upper	[A-Z]
Lettre	[a-zA-Z]
LC 	({Lettre}|{Chiffre})
Delim   [ \t\n\r]
Symbole [&+\-*/();:,.{}]
BS      \\.
PasBSG [^\\"]
Guill		"\""
Star  		"*"
NotStar		[^*]
NotStarPar      [^*/]

%{
#include <string.h>
#include <strings.h>
#include "tp.h"
#include "tp_y.h"

extern char *strdup(char *);
extern YYSTYPE yylval;
extern int yylineno;
extern int yylex();
%}

%%
{Entier} 	{ yylval.I = atoi(yytext); return(Cste); }
{Symbole}	{ return(yytext[0]); }
":="		{ return(AFF); }
"="		{ yylval.C = EQ; return(RelOp); }
"<>"		{ yylval.C = NE; return(RelOp); }
"<="		{ yylval.C = LE; return(RelOp); }
">="		{ yylval.C = GE; return(RelOp); }
">"		{ yylval.C = GT; return(RelOp); }
"<"		{ yylval.C = LT; return(RelOp); }
%%
