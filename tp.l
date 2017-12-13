Chiffre 	[0-9]
Entier 		{Chiffre}+
Lower		[a-z]
Upper		[A-Z]
Lettre		[a-zA-Z]
LC 			({Lettre}|{Chiffre})
Delim   	[ \t\n\r]
Symbole 	[&]
BS      	\\.
PasBSG 		[^\\"]
Guill		"\""
Star  		"*"
NotStar		[^*]
NotStarPar  [^*/]
StrChar		[^\"\n]
InterComm	{NotStar}|{Star}*{NotStarPar}*
Comm1		"/*"{InterComm}"*/"
Comm2		"//"{StrChar}*"\n"

%{
#include <string.h>
#include <strings.h>
#include "tp.h"
#include "tp_y.h"

extern char *strdup(char *);
extern YYSTYPE yylval;
extern int yylineno;
extern int yylex();

int motClef(){
	if( strcmp("class",yytext) == 0){return CLASS;}
	if( strcmp("extends",yytext) == 0){return EXTENDS;}
	if( strcmp("is",yytext) == 0){return IS;}
	if( strcmp("var",yytext) == 0){return VAR;}
	if( strcmp("object",yytext) == 0){return OBJECT;}
	if( strcmp("override",yytext) == 0){return OVERRIDE;}
	if( strcmp("def",yytext) == 0){return DEF;}
	if( strcmp("if",yytext) == 0){return IF;}
	if( strcmp("then",yytext) == 0){return THEN;}
	if( strcmp("else",yytext) == 0){return ELSE;}
	
	yylval.S = strdup(yytext);
	return Id;
}

%}

%%
{Entier} 			{ yylval.I = atoi(yytext); return(Cste); }
{Symbole}			{ return(yytext[0]); }
{Lower}{LC}*		{ return motClef();}
{Upper}{LC}*		{ yylval.S = strdup(yytext); return(IdClass); }
":="				{ return(AFF); }

"="					{ yylval.C = EQ; return(RelOp); }
"<>"				{ yylval.C = NE; return(RelOp); }
"<="				{ yylval.C = LE; return(RelOp); }
">="				{ yylval.C = GE; return(RelOp); }
">"					{ yylval.C = GT; return(RelOp); }
"<"					{ yylval.C = LT; return(RelOp); }

"+"					{ return(ADD); }
"-"					{ return(MIN); }
"/"					{ return(DIV); }
"*"					{ return(MUL); }
"("					{ return('('); }
")"					{ return(')'); }
";"					{ return(';'); }
"."					{ return('.'); }
"{"					{ return('{'); }
"}"					{ return('}'); }
":"					{ return(':'); }
","					{ return(','); }
{Guill}				{ return('"'); }


{Comm1}|{Comm2}		{}
%%
