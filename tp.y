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

%type<pinit> Init
%type<pbloc> ListInstClause ListInst
%type<pT> Inst ITE RETURN cible ExprRelop Champ Expr Object Selection CallMethod Instanciation Cast


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
| 						{ $$ = makeInit(NULL);}
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
Champ : VAR ID ':' ID Init ';' 	{   id1 = makeLeafId(_ID, $2);
                                    id2 = makeLeafId(_ID, $4);
                                    $$ = makeTree(I_CHAMP, 3, id1, id2, $5);}
;
////////////////////////////////

//Appel d'une methode

CallMethod : Object'.'ID'('ListArgClause')'	{ $$ = makeTree(E_CALL_METHOD, 3,  $1, $3, $5);}
| '('ExprRelop')''.'ID'('ListArgClause')'	{ $$ = makeTree(E_CALL_METHOD, 3,  $2, $5, $7);}
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
| block 			{ $$ = makeTree(0, $1);}
| RETURN ';'		/*{ $$ = makeLeafId(_ID, NULL(t_variable));}*/
| cible ';'			{ $$ = $1;}
| ExprRelop ';' 	{ $$ = makeTree(I_EXPRRELOP, 1, $1);}
| Champ				{ $$ = $1;}
;

ITE : IF ExprRelop THEN Inst ELSE Inst { $$ = makeTree(I_ITE, 3, $2, $4, $6);}
;

cible : Object AFF ExprRelop	{ $$ = makeTree(I_AFF, 2, $1, $3);}
;	

Selection : Object'.'ID		{ $$ = makeExprSelect($3, $1);}
| '('ExprRelop')''.'ID		{ $$ = makeExprSelect($5, $2);}
;

Object : Selection		{ $$ = $1;}
| CallMethod			{ $$ = $1;}
| Instanciation			{ $$ = $1;}
| CSTE					{ $$ = makeExprCste(8, $1);}
| ID					{ $$ = makeLeafId(_ID, $1);	}
| Cast					{ $$ = $1;}
;	


ExprRelop : Expr RELOP Expr	{ $$ = makeExpr($2, $1, $3);}
| Expr						{ $$ = $1;}
;

Expr : Expr ADD Expr		{ $$ = makeExpr(SUM, $1, $3);}
| Expr SUB Expr				{ $$ = makeExpr(MIN, $1, $3);}
| Expr MUL Expr				{ $$ = makeExpr(MULT, $1, $3);}
| Expr DIV Expr				{ $$ = makeExpr(DIVI, $1, $3);}
| Expr '&' Expr				{ $$ = makeExpr(AND, $1, $3);}
| Object					{ $$ = $1;}
| ADD CSTE %prec UNAIRE		{   cste = makeLeafInt(_CSTE, $2)
                                $$ = makeTree(SUM, 1, cste);}
| SUB CSTE %prec UNAIRE		{   cste = makeLeafInt(_CSTE, $2)
                                $$ = makeTree(MIN, 1, cste);}
| '(' ExprRelop ')'	 		{ $$ = $2;}
;

Instanciation : NEWV ID '('ListArgClause')'		{ $$ = makeTree(E_INST, 2, $2, $4);}
;

Cast : '('ID Object')' 	{   id = makeLeafId(_ID, $2);
                            $$ = makeTree(E_CAST, 2, id, $3);}
;
