/* attention: NEW est defini dans tp.h Utilisez un autre nom de token */
%token IS OBJECT CLASS VAR EXTENDS DEF OVERRIDE IF THEN ELSE AFF RETURN NEWV 
%token ',' ':' '(' ')' '{' '}' ';' '.' '&' 
%token ADD SUB MUL DIV
%token<S> ID
%token<I> CSTE
%token<C> RELOP

%right '.'

%left '&'
%left ADD SUB
%left MUL DIV
%left UNAIRE

%type<pT> Inst ITE RETURN cible ExprRelop Champ Expr Object Selection CallMethod Instanciation 
%type<pT> Cast Param ListParam ListParamClause ListArgClause Init ListInstClause ListInst ListArg Arg 
%type<pT> listClassObj constructorClause extendsClause ListVarDef Var VAR block Prog


%{
#include "tp.h"
#include "tp_y.h"

extern int yylex();
extern void yyerror(char *);
%}

%%
//Structure du programme
Prog : listClassObj block 	{ $$ = makeTree(PROG, 2, $1, $2);}
;	

//Gestion des Objets et Classes
listClassObj : listClassObj ClassObj
| 
;

ClassObj : declClass
| declObject
;

declClass : CLASS ID '(' ListParamClause ')' extendsClause constructorClause IS classObjBlock
;

declObject : OBJECT ID IS classObjBlock
;

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

Init : AFF ExprRelop
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
Champ : VAR ID ':' ID Init ';' 	{   TreeP id1 = makeLeafId(_ID, $2);
									TreeP id2 = makeLeafId(_ID, $4);
									$$ = makeTree(I_CHAMP, 3, id1, id2, $5);}
;
////////////////////////////////

//Appel d'une methode

CallMethod : Object'.'ID'('ListArgClause')'	{ $$ = makeTree(E_CALL_METHOD, 3,  $1, $3, $5);}
| '('ExprRelop')''.'ID'('ListArgClause')'	{ $$ = makeTree(E_CALL_METHOD, 3,  $2, $5, $7);}
;

block: '{' ListInstClause '}'	{ TreeP decls = makeLeafLVar(DECL, NULL);
								  $$ = makeTree(I_BLOC, 2, decls, $2); }
| '{' ListChamp IS ListInst '}'	/*{ TreeP decls = makeLeafLVar(DECL, $);*/
								/*  $$ = makeTree(I_BLOC, 2, decls, $2); }*/
;

ListChamp : ListChamp Champ
| Champ
;

ListInstClause : ListInst	
| 
;

ListInst : Inst ListInst 	{ $$ = makeTree(INST, 2, $1, $2);}
| Inst						{ $$ = makeTree(INST, 1, $1); }
;

Inst : ITE 			{ $$ = $1;}
| block 			{ $$ = $1;}
| RETURN ';'		/*{ $$ = makeLeafId(_ID, NULL(t_variable));}*/
| cible ';'			{ $$ = $1;}
| ExprRelop ';' 	{ $$ = makeTree(I_EXPRRELOP, 1, $1);}
| Champ				{ $$ = $1;}
;

ITE : IF ExprRelop THEN Inst ELSE Inst { $$ = makeTree(I_ITE, 3, $2, $4, $6);}
;

cible : Object AFF ExprRelop	{ $$ = makeTree(I_AFF, 2, $1, $3);}
;	

Selection : Object'.'ID	
| '('ExprRelop')''.'ID
;

Object : Selection		{ $$ = $1;}
| CallMethod			/*{ $$ = $1;}*/
| Instanciation			{ $$ = $1;}
| CSTE					{ $$ = makeLeafInt(CST, $1);}
| ID					{ $$ = makeLeafId(_ID, $1);	}
| Cast					{ $$ = $1;}
;	


ExprRelop : Expr RELOP Expr	{ $$ = makeTree($2, 2, $1, $3);}
| Expr						{ $$ = $1;}
;

Expr : Expr ADD Expr		{ $$ = makeTree(SUM, 2, $1, $3);}
| Expr SUB Expr				{ $$ = makeTree(MIN, 2, $1, $3);}
| Expr MUL Expr				{ $$ = makeTree(MULT, 2, $1, $3);}
| Expr DIV Expr				{ $$ = makeTree(DIVI, 2, $1, $3);}
| Expr '&' Expr				{ $$ = makeTree(AND, 2, $1, $3);}
| Object					{ $$ = $1;}
| ADD CSTE %prec UNAIRE		{ TreeP cste = makeLeafInt(CST, $2);
							  $$ = makeTree(SUM, 1, cste);	}
| SUB CSTE %prec UNAIRE		{ TreeP cste = makeLeafInt(CST, $2);
							  $$ = makeTree(SUM, 1, cste);	}
| '(' ExprRelop ')'	 		{ $$ = $2;}
;

Instanciation : NEWV ID '('ListArgClause')'		{ $$ = makeTree(INST, 2, $2, $4);}
;

Cast : '('ID Object')' 	{   TreeP id = makeLeafId(_ID, $2);
							$$ = makeTree(CAST, 2, id, $3);}
;
