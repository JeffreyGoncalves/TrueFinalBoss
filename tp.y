/* attention: NEW est defini dans tp.h Utilisez un autre nom de token */
%token IS OBJECT CLASS VAR EXTENDS DEF OVERRIDE IF THEN ELSE AFF RETURN NEWV
%token ',' ':' '(' ')' '{' '}' ';' '.'
%token ADD SUB MUL DIV
%token<S> ID
%token<I> CSTE
%token<C> RELOP

%right '.'

%left ADD SUB
%left MUL DIV

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
| ;

ClassObj : declClass
| declObject;

declClass : CLASS ID '(' ListParamClause ')' extendsClause constructorClause IS classObjBlock
;

declObject : OBJECT ID IS classObjBlock;

constructorClause : block		
| ;

////// Parametres //////
ListParamClause : ListParam
| ;

ListParam : Param ',' ListParam 
| Param ;

Param : Var ID':' ID Init
;

Var : VAR
| ;

Init : AFF ExprRelop
| ;
/////////////////////////

// Mot cle EXTENDS //
extendsClause : EXTENDS ID '(' ListArgClause ')'
| ;

ListArgClause : ListArg
| ;

ListArg : Arg ',' ListArg 
| Arg;

Arg : ExprRelop
;
////////////////////

// Bloc d'un objet ou classe //
classObjBlock : '{' ListVarDef '}'

ListVarDef : VarDef ListVarDef
| ;

VarDef : declMethod 
| Champ ;
///////////////////////////////

// Declaration d'une methode //

declMethod : Override DEF ID'(' ListParamClause ')' ':' ID AFF ExprRelop
| Override DEF ID'(' ListParamClause ')' ClassClause IS block;

Override : OVERRIDE
| ;

ClassClause : ':' ID
| ;
//////////////////////////////

//Champ dans un objet
Champ : VAR ID ':' ID Init ';' ;
////////////////////////////////

//Appel d'une methode

CallMethod : Object'.'ID'('ListArgClause')'
| CSTE'.'ID'('ListArgClause')'
;

block: '{' ListInstClause '}';

ListInstClause : ListInst
| ;

ListInst : Inst ListInst 
| Inst;

Inst : ITE 
| block 
| RETURN ';'
| cible ';'
| ExprRelop ';' 
| Champ
;

ITE : IF ExprRelop THEN Inst ELSE Inst 
;

cible : Object AFF ExprRelop ;

Way : ID
| Object'.'ID
;

Object : Way
| CallMethod
;

ExprRelop : Expr RELOP Expr
| Expr
;

Expr : Expr ADD Expr
| Expr SUB Expr
| Expr MUL Expr
| Expr DIV ExprS
| Instanciation
| Object
| CSTE
| Champ
| '('ExprRelop')'
;


Instanciation : NEWV ID '('ListArgClause')'

