#include "fonctions.h"

extern int yyparse();
extern int yylineno;

/*typedef struct t_class{
	char* name;
	struct t_method** constructor;
	struct t_method** methods;
	struct VarDecl* attributes;
	struct t_class* superClass;
}t_class;*/

/* REMPLISSAGE STRUCT DE CLASSE */
t_class* makeListClass(TreeP TreeClass){
	
	if(List != NIL(Tree)){
		myClass = NEW(1, t_class);
		
		/*LE NOM*/
		myClass->name = getChild(TreeClass, 0)->u.str;
		
		/*LA LISTE de PARAMETRES*/
		
	}else{
		return NIL(t_class);
	}
}

t_method* DMtoS(TreeP Tree){

	t_method* method = NEW(1,t_method);
	if(Tree->op == DECL_METH){
		method->name = getChild(Tree,0)->u.str;
		if(Tree->nbChildren == 3){					/*cas DeclMethod ::= Override DEF ID'(' ListParamClause ')' ':' ID AFF ExprRelop*/
			if(getChild(Tree,2) == NULL){
				method->isReDef == false;	
			}
			else{method->isReDef == true;}	
		}
		else{

		}
	}


}
