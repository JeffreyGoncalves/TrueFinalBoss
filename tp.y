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

%type<pT> Inst ITE RETURN cible ExprRelop Champ Expr Object Selection CallMethod Instanciation declClass declObject
%type<pT> Cast ListParam ListParamClause ListArgClause Init ListInstClause ListInst ListArg Arg ClassObj classObjBlock
%type<pT> listClassObj constructorClause extendsClause ListVarDef Var block Prog ListChamp Override ClassClause
%type<pV> Param


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
listClassObj : listClassObj ClassObj	{ $$ = makeTree(PROG, 2, $1, $2);}
| 										{ $$ = NIL(Tree);}
;

ClassObj : declClass					{ $$ = makeTree(CLAS, 1, $1); }
| declObject							{ $$ = makeTree(OBJ, 1, $1); }
;

declClass : CLASS ID '(' ListParamClause ')' extendsClause constructorClause IS classObjBlock 
{ TreeP id = makeLeafStr(_ID, $2);
  $$ = makeTree(_CLASS,5,id,$2,$4,$6,$7,$9);}
;

declObject : OBJECT ID IS classObjBlock
;

constructorClause : block		
| 									{$$ = NIL(Tree);}
;

////// Parametres //////
ListParamClause : ListParam			{ $$ = $1;}
| 									{ $$ = NIL(Tree);}
;

ListParam : Param ',' ListParam 		{ TreeP list = $3;
							ajouteParam(list, $1);
							$$ = list;}
| Param 					{ $$ = makeLeafParam(LIST_PARAM, $1);}
;

Param : Var ID':' ID Init			{ $$ = makeVarDeclP($2, $4);}
;

Var : VAR				{ $$ = makeLeafStr(_VAR, "var");}
| 						{ $$ = NIL(Tree);}
;

Init : AFF ExprRelop	{ $$ = $2; }
| 						{ $$ = NIL(Tree);}
;
/////////////////////////

// Mot cle EXTENDS //
extendsClause : EXTENDS ID '(' ListArgClause ')'	{	TreeP id = makeLeafStr(_ID, $2);
														$$ = makeTree(_EXTENDS, 2, id, $4);}
| 													{ $$ = NIL(Tree);}
;

ListArgClause : ListArg			{ $$ = $1;}
| 								{ $$ = NIL(Tree);}
;

ListArg : Arg ',' ListArg		{ $$ = makeTree(LIST_ARG, 2,  $1, $3);}
| Arg							{ $$ = makeTree(LIST_ARG, 2,  $1, NIL(Tree));}
;

Arg : ExprRelop				{ $$ = $1;}
;
////////////////////

// Bloc d'un objet ou classe //
classObjBlock : '{' ListVarDef '}'   {$$ = $2;}
;

ListVarDef : VarDef ListVarDef
| 				{ $$ = NIL(Tree);}
;

VarDef : declMethod
| Champ
;
///////////////////////////////

// Declaration d'une methode //
declMethod : Override DEF ID'(' ListParamClause ')' ':' ID AFF ExprRelop
| Override DEF ID'(' ListParamClause ')' ClassClause IS block
;

Override : OVERRIDE		{ $$ = makeLeafStr(_OVERRIDES, "override");}
| 				{ $$ = NIL(Tree);}
;

ClassClause : ':' ID	{ $$ = makeLeafStr(CLASS_NAME, $2);}
| 						{ $$ = NIL(Tree);}					
;
//////////////////////////////

//Champ dans un objet
Champ : VAR ID ':' ID Init ';' 	{   TreeP id1 = makeLeafStr(_ID, $2);
									TreeP id2 = makeLeafStr(_ID, $4);
									$$ = makeTree(I_CHAMP, 3, id1, id2, $5);}
;
////////////////////////////////

//Appel d'une methode

CallMethod : Object'.'ID'('ListArgClause')'	{ $$ = makeTree(E_CALL_METHOD, 3,  $1, $3, $5);}
| '('ExprRelop')''.'ID'('ListArgClause')'	{ $$ = makeTree(E_CALL_METHOD, 3,  $2, $5, $7);}
;

block: '{' ListInstClause '}'	{ TreeP decls = makeLeafLVar(DECL, NIL(VarDecl));
								  $$ = makeTree(I_BLOC, 2, decls, $2); }
| '{' ListChamp IS ListInst '}'	/*{ TreeP decls = makeLeafLVar(DECL, $);*/
								/*  $$ = makeTree(I_BLOC, 2, decls, $2); }*/
;

ListChamp : ListChamp Champ	{ $$ = makeTree(INST, 2, $1, $2);}
| Champ				{ $$ = $1;}
;

ListInstClause : ListInst	{ $$ = $1;}
| 				{ $$ = NIL(Tree);}
;

ListInst : Inst ListInst 	{ $$ = makeTree(INST, 2, $1, $2);}
| Inst				{ $$ = makeTree(INST, 1, $1); }
;

Inst : ITE 			{ $$ = $1;}
| block 			{ $$ = $1;}
| RETURN ';'		/*{ $$ = makeLeafStr(_ID, NIL(Tree)(t_variable));}*/
| cible ';'			{ $$ = $1;}
| ExprRelop ';' 	{ $$ = makeTree(I_EXPRRELOP, 1, $1);}	
;

ITE : IF ExprRelop THEN Inst ELSE Inst { $$ = makeTree(I_ITE, 3, $2, $4, $6);}
;

cible : Object AFF ExprRelop	{ $$ = makeTree(I_AFF, 2, $1, $3);}
;	

Selection : Object'.'ID		{ TreeP id = makeLeafStr(_ID, $3);
					$$ = makeTree(E_SELECT, 2, $1, id);	}
| '('ExprRelop')''.'ID		{ TreeP id = makeLeafStr(_ID, $5);
					$$ = makeTree(E_SELECT, 2, $2, id);	}
;

Object : Selection		{ $$ = $1;}
| CallMethod			{ $$ = $1;}
| Instanciation			{ $$ = $1;}
| CSTE					{ $$ = makeLeafInt(CST, $1);}
| ID					{ $$ = makeLeafStr(_ID, $1);	}
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

Cast : '('ID Object')' 	{   TreeP id = makeLeafStr(_ID, $2);
							$$ = makeTree(CAST, 2, id, $3);}
;
