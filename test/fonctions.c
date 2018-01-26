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

TreeP makeListClass(TreeP List){
	
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
