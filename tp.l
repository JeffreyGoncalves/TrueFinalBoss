Chiffre 	[0-9]
Entier 		{Chiffre}+
Lower		[a-z]
Upper		[A-Z]
Lettre		[a-zA-Z]
LC 			({Lettre}|{Chiffre})
Delim   	[ \t\n\r]
Symbole 	[&]
BS      	\\.
PasBSG 		({PasGuill}|"\\".)
PasGuill	[^"]
Guill		"\""
Star  		"*"
NotStar		[^*]
NotStarPar  [^*/]
StrChar		[^\n]
InterComm	{NotStar}|{Star}*{NotStarPar}
Comm1		"/*"{InterComm}*"*/"
Comm2		"//"{StrChar}*"\n"
ChaineCarac	{Guill}{PasBSG}*{Guill}
And			"&"

%{
#include <string.h>
#include <strings.h>
#include "tp.h"
#include "tp_y.h"

extern char *strdup(char *);
extern YYSTYPE yylval;
extern int yylineno;
extern int yylex();

void lexError(char c) {
  fprintf(stderr, "Error: Skipping invalid character '%c' on line %d\n",
          c, yylineno);
  setError(LEXICAL_ERROR);
}

void removeComma(char *str) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != '"')
        {
			if(*dst == '\\' && *(src+1) == '"')
			{
				src++;
				*dst = *src;
			}
			dst ++;
		}
    }
    *dst = '\0';
}

int motClef(){
	if( strcmp("class",yytext) == 0){return CLASS;}
	if( strcmp("extends",yytext) == 0){return EXTENDS;}
	if( strcmp("is",yytext) == 0){return IS;}
	if( strcmp("var",yytext) == 0){return VAR;}
	if( strcmp("new",yytext) == 0){return NEWV;}
	if( strcmp("object",yytext) == 0){return OBJECT;}
	if( strcmp("override",yytext) == 0){return OVERRIDE;}
	if( strcmp("def",yytext) == 0){return DEF;}
	if( strcmp("if",yytext) == 0){return IF;}
	if( strcmp("then",yytext) == 0){return THEN;}
	if( strcmp("else",yytext) == 0){return ELSE;}
	if( strcmp("return",yytext) == 0){return RETURN;}
	
	yylval.S = strdup(yytext);
	return ID;
}

%}

%%
{Entier} 			{ yylval.I = atoi(yytext); return(CSTE); }
{Lettre}{LC}*		{ return motClef();}
":="				{ return(AFF); }
{ChaineCarac}		{ char *word = strdup(yytext);
					  removeComma(word);
					  yylval.S = word;
					  return(STR); }

"="					{ yylval.C = EQ; return(RELOP); }
"<>"				{ yylval.C = NE; return(RELOP); }
"<="				{ yylval.C = LE; return(RELOP); }
">="				{ yylval.C = GE; return(RELOP); }
">"					{ yylval.C = GT; return(RELOP); }
"<"					{ yylval.C = LT; return(RELOP); }
"&"					{ return('&'); }
"+"					{ return(ADD); }
"-"					{ return(SUB); }
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


{Comm1}|{Comm2}|{Delim}*		{}

.					{lexError(yytext[0]);}
%%
