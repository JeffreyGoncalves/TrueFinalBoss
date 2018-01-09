/* attention: NEW est defini dans tp.h Utilisez un autre nom de token */
%token IS OBJECT CLASS VAR EXTENDS DEF OVERRIDE IF THEN ELSE AFF RETURN CHAINECRACTERE
%token ',' ':' '(' ')' '{' '}' ';' '.'
%token ADD SUB MUL DIV
%token<S> ID CLASSID
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
Prog : classLOpt block;

///////////////////////////////////////////////////////////////////////////////////////////
//Declaration d'une classe

declClass : CLASS CLASSID '(' ListParamClause ')' extendsClause constructorClause IS block
;

ListParamClause : ListParam
| ;

ListParam : Param ',' ListParam 
| Param ;

Param :
| Var Way':' CLASSID Init
| Var CallMethod':' CLASSID Init
;

Var : VAR
| ;

Init : AFF Expr
| ;

extendsClause : EXTENDS CLASSID '(' ListArgClause ')'
;

ListArgClause : ListArg
| ;

ListArg : Arg ',' ListArg 
| Arg;

Arg : Expr	 					//A voir (cas new Point(4,5))
;

constructorClause : block		//?
| ;
///////////////////////////////////////////////////////////////////////////////////////////
//Declaration d'objet isole

declObject : OBJECT ID IS block //?
;
///////////////////////////////////////////////////////////////////////////////////////////
//Declaration d'une methode


declMethod : Override DEF ID'(' ListParamClause ')' ':' ID ADD Expr
| Override DEF ID'(' ListParamClause ')' ClassClause IS block

Override : OVERRIDE
| ;

ClassClause : ':' CLASSID
| ;

//Appel d'une methode

CallMethod : CallMethod'.'ID'('ListArgClause')'
| Way'.'ID'('ListArgClause')'
| CSTE'.'ID'('ListArgClause')'
;
///////////////////////////////////////////////////////////////////////////////////////////

classLOpt: ListDeclClass
| ;

ListDeclClass : declClass ListDeclClass
| declClass;

block: '{' ListInstClause '}';

ListInstClause : ListInst
| ;

ListInst : Inst ListInst 
| Inst;

Inst : ITE 
| declObject
| declMethod
| block 
| RETURN ';'
| cible ';'
| Expr ';' ;

ITE : IF Expr THEN Inst ELSE Inst 
;

cible : Way AFF Expr ;

Way : ID
| Way'.'ID
| CallMethod'.'ID
;

Object : Way
| CallMethod
;

Expr : Expr ADD Expr
| Expr SUB Expr
| Expr MUL Expr
| Expr DIV Expr
| Expr RELOP Expr
| '('Expr')'
| Object
| CSTE
;

