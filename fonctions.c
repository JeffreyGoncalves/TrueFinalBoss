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

	t_method* forest = NEW(1,t_method);
	if(Tree->op == DECL_METH){
		if(Tree->nbChildren == 3){

		}
		else{

		}
	}


}
