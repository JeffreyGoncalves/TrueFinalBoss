/* attention: NEW est defini dans tp.h Utilisez un autre nom de token */
%token IS OBJECT CLASS VAR EXTENDS DEF OVERRIDE IF THEN ELSE AFF RETURN NEWV 
%token ',' ':' '(' ')' '{' '}' ';' '.' '&' 
%token ADD SUB MUL DIV
%token<pvariable> ID
%token<pvalue> CSTE
%token<C> RELOP

%right '.'

%left '&'
%left ADD SUB
%left MUL DIV
%left UNAIRE

%type<paffect> cible
%type<pinstr> Inst ITE block RETURN
%type<pexpr> Object Expr ExprRelop Arg Cast Selection
%type<pinstanciation> Instanciation
%type<pinit> Init
%type<pchamp> Champ


%{
#include "tp.h"
#include "tp_y.h"

extern int yylex();
extern void yyerror(char *);
%}

%%
//Structure du programme
Prog : listClassObj block;

//Gestion des Objets et Classes
listClassObj : listClassObj ClassObj
| 
;

ClassObj : declClass
| declObject
;

declClass : CLASS ID '(' ListParamClause ')' extendsClause constructorClause IS classObjBlock
;

declObject : OBJECT ID IS classObjBlock;

constructorClause : block		
| 
;

////// Parametres //////
ListParamClause : ListParam
| 
;

ListParam : Param ',' ListParam 
| Param 
;

Param : Var ID':' ID Init
;

Var : VAR
| 
;
Init : AFF ExprRelop	{ $$ = makeInit($2);}
| 
;
/////////////////////////

// Mot cle EXTENDS //
extendsClause : EXTENDS ID '(' ListArgClause ')'
| 
;

ListArgClause : ListArg
| 
;

ListArg : Arg ',' ListArg 
| Arg
;

Arg : ExprRelop
;
////////////////////

// Bloc d'un objet ou classe //
classObjBlock : '{' ListVarDef '}'
;

ListVarDef : VarDef ListVarDef
| 
;

VarDef : declMethod 
| Champ 
;
///////////////////////////////

// Declaration d'une methode //
declMethod : Override DEF ID'(' ListParamClause ')' ':' ID AFF ExprRelop
| Override DEF ID'(' ListParamClause ')' ClassClause IS block
;

Override : OVERRIDE
| 
;

ClassClause : ':' ID
| 
;
//////////////////////////////

//Champ dans un objet
Champ : VAR ID ':' ID Init ';' 	{ $$ = makeChamp($2, $4);}
;
////////////////////////////////

//Appel d'une methode

CallMethod : Object'.'ID'('ListArgClause')'
| '('ExprRelop')''.'ID'('ListArgClause')'
;

block: '{' ListInstClause '}'
| '{' ListInst IS ListInst '}'
;

ListInstClause : ListInst
| 
;

ListInst : Inst ListInst 
| Inst
;

Inst : ITE 			{ $$ = $1;}
| block 			{ $$ = makeInstruction(0, $1);}
| RETURN ';'			{ $$ = makeInstruction(1, $1);}
| cible ';'			{ $$ = makeInstruction(2, $1);}
| ExprRelop ';' 		{ $$ = makeInstruction(4, $1);}
| Champ				{ $$ = makeInstruction(4, $1);}
;

ITE : IF ExprRelop THEN Inst ELSE Inst { $$ = makeInstruction(3, $2, $4, $6);}
;

cible : Object AFF ExprRelop	{ $$ = makeAff($1, $3);}
;	

Selection : Object'.'ID		{ $$ = makeExprSelect($3, $1);}
| '('ExprRelop')''.'ID		{ $$ = makeExprSelect($5, $2);}
;

Object : Selection		{ $$ = $1;}
| CallMethod			/*{ $$ = makeExprVar($1);}*/
| Instanciation			{ $$ = makeExprInst($1);}
| CSTE				{ $$ = makeExprCste(8, $1);}
| ID				{ $$ = makeExprVar($1);}		
| Cast				{ $$ = $1;}
;


ExprRelop : Expr RELOP Expr	/*{ $$ = makeExpr($2, $1, $3);}*/
| Expr				{ $$ = $1;}
;

Expr : Expr ADD Expr		{ $$ = makeExpr(SUM, $1, $3);}
| Expr SUB Expr			{ $$ = makeExpr(MIN, $1, $3);}
| Expr MUL Expr			{ $$ = makeExpr(MULT, $1, $3);}
| Expr DIV Expr			{ $$ = makeExpr(DIVI, $1, $3);}
| Expr '&' Expr			{ $$ = makeExpr(AND, $1, $3);}
| Object			{ $$ = $1;}
| ADD CSTE %prec UNAIRE		{ $$ = makeExprCste(1, $2);}
| SUB CSTE %prec UNAIRE		{ $$ = makeExprCste(2, $2);}
| '(' ExprRelop ')'		{ $$ = $2;}
;

Instanciation : NEWV ID '('ListArgClause')'
;

Cast : '('ID Object')'
;
