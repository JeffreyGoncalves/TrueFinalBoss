/* attention: NEW est defini dans tp.h Utilisez un autre nom de token */
%token IS OBJECT CLASS VAR EXTENDS DEF OVERRIDE IF THEN ELSE AFF RETURN NEWV 
%token ',' ':' '(' ')' '{' '}' ';' '.' '&' 
%token ADD SUB MUL DIV
%token<S> ID STR
%token<I> CSTE
%token<C> RELOP

%right '.'

%left '&'
%left ADD SUB
%left MUL DIV
%left UNAIRE

%type<pT> Inst ITE RETURN cible ExprRelop Expr Object Selection CallMethod Instanciation declClass declObject VarDef
%type<pT> Cast ListParam ListParamClause ListArgClause Init ListInstClause ListInst ListArg Arg ClassObj classObjBlock
%type<pT> listClassObj constructorClause extendsClause ListVarDef Var block Prog ListChamp Override ClassClause declMethod
%type<pV> Param Champ


%{
/*#include "tp.h"*/
#include "fonctions.h"
#include "tp_y.h"

extern int yylex();
extern void yyerror(char *);
%}

%%
//Structure du programme
Prog : listClassObj block 				{ $$ = makeTree(PROG, 2, $1, $2);
										  affTree($$, 0);
										  compile($1,$2);			
										  }
;	

//Gestion des Objets et Classes
listClassObj : listClassObj ClassObj	{ $$ = makeTree(LIST_CLASS, 2, $1, $2);}
| 										{ $$ = NIL(Tree);}
;

ClassObj : declClass					{ $$ = makeTree(CLAS, 1, $1); }
| declObject							{ $$ = makeTree(OBJ, 1, $1); }
;

declClass : CLASS ID '(' ListParamClause ')' extendsClause constructorClause IS classObjBlock 
{ TreeP id = makeLeafStr(_ID, $2);
  $$ = makeTree(_CLASS,5,id,$4,$6,$7,$9);
  }
;

declObject : OBJECT ID IS classObjBlock { 	TreeP id = makeLeafStr(_ID, $2);
											$$ = makeTree(DECLA_OBJECT, 2, id, $4); }
;

constructorClause : block			{ $$ = $1;}
| 									{$$ = NIL(Tree);}
;

////// Parametres //////
ListParamClause : ListParam			{ $$ = $1;
										/*afficheParam($$->u.lvar);*/
									}
| 									{ $$ = NIL(Tree);}
;

ListParam : Param ',' ListParam 		{ TreeP list = $3;
										  ajouteParam(list, $1);
							              $$ = list;}
| Param 					{ $$ = makeLeafParam(LIST_PARAM, $1);}
;

Param : Var ID':' ID Init			{ $$ = makeVarDeclP($2, $4, $5);}
;

Var : VAR				{ $$ = makeLeafStr(_VAR, "var");}
| 						{ $$ = NIL(Tree);}
;

Init : AFF ExprRelop	{ $$ = $2; }
| 						{ $$ = NIL(Tree);}
;
/////////////////////////

// Mot cle EXTENDS //
extendsClause : EXTENDS ID '(' ListArgClause ')'	{ TreeP id = makeLeafStr(_ID, $2);
													  $$ = makeTree(_EXTENDS, 2, id, $4);}
| 													{ $$ = NIL(Tree);}
;

ListArgClause : ListArg			{ $$ = $1;}
| 								{ $$ = NIL(Tree);}
;

ListArg : Arg ',' ListArg		{ $$ = makeTree(LIST_ARG, 2,  $1, $3);}
| Arg							{ $$ = makeTree(LIST_ARG, 2,  $1, NIL(Tree));}
;

Arg : ExprRelop					{ $$ = $1;}
;
////////////////////

// Bloc d'un objet ou classe //
classObjBlock : '{' ListVarDef '}'   {$$ = $2;}
;

ListVarDef : VarDef ListVarDef		{ $$ = makeTree(LIST_VAR_DEF, 2,  $1, $2);}
| 									{ $$ = NIL(Tree);}
;

VarDef : declMethod					{ $$ = makeTree(VAR_DEF_METH, 1,  $1);}
| Champ								{ $$ = makeLeafLVar(VAR_DEF_CHAMP,  $1);}
;
///////////////////////////////

// Declaration d'une methode //
declMethod : Override DEF ID'(' ListParamClause ')' ':' ID AFF ExprRelop { TreeP id1 = makeLeafStr(_ID,$3);
																		   TreeP id2 = makeLeafStr(_ID,$8);
																		   $$ = makeTree(DECL_METH_1,5,id1,id2,$1,$5,$10);}
| Override DEF ID'(' ListParamClause ')' ClassClause IS block            { TreeP id = makeLeafStr(_ID,$3);
																		   $$ = makeTree(DECL_METH_2,5,id,$1,$5,$7,$9);}
;

Override : OVERRIDE		{ $$ = makeLeafStr(_OVERRIDE, "override");}
| 						{ $$ = NIL(Tree);}
;

ClassClause : ':' ID	{ $$ = makeLeafStr(CLASS_NAME, $2);}
| 						{ $$ = NIL(Tree);}					
;
//////////////////////////////

//Champ dans un objet
Champ : VAR ID ':' ID Init ';' 	  { $$ = makeVarDeclP($2,$4,$5);}
;
////////////////////////////////

//Appel d'une methode

CallMethod : Object'.'ID'('ListArgClause')'	{ 	TreeP id = makeLeafStr(_ID, $3);
												$$ = makeTree(E_CALL_METHOD, 3,  $1, id, $5);}
| '('ExprRelop')''.'ID'('ListArgClause')'	{ 	TreeP id = makeLeafStr(_ID, $5);
												$$ = makeTree(E_CALL_METHOD, 3,  $2, id, $7);}
;

block: '{' ListInstClause '}'	{ TreeP decls = makeLeafLVar(DECL, NIL(VarDecl));
								  $$ = makeTree(I_BLOC, 2, decls, $2); }
| '{' ListChamp IS ListInst '}'	{ $$ = makeTree(I_BLOC, 2, $2, $4); }
;

ListChamp : ListChamp Champ		{ TreeP list = $1;
								ajouteParam(list, $2);
							    $$ = list;}
| Champ						{ $$ = makeLeafParam(LIST_PARAM, $1);}
;

ListInstClause : ListInst	{ $$ = $1;}
| 							{ $$ = NIL(Tree);}
;

ListInst : Inst ListInst 	{ $$ = makeTree(INST, 2, $1, $2);}
| Inst						{ $$ = makeTree(INST, 1, $1); }
;

Inst : ITE 			{ $$ = $1;}
| block 			{ $$ = $1;}
| RETURN ';'		{ $$ = makeLeafStr(I_RETURN, "result");}
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
| ID					{ $$ = makeLeafStr(_ID, $1);}
| STR					{ $$ = makeLeafStr(_STR, $1);}
| Cast					{ $$ = $1;}
;	


ExprRelop : Expr RELOP Expr	{ $$ = makeTree(EXPR_RELOP, 2, $1, $3);}
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

Instanciation : NEWV ID '('ListArgClause')'		{ 	TreeP id = makeLeafStr(_ID, $2);
													$$ = makeTree(INST, 2, id, $4);}
;

Cast : '('ID Object')' 		{ TreeP id = makeLeafStr(_ID, $2);
							  $$ = makeTree(CAST, 2, id, $3);}
;
