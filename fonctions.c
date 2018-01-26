#include "fonctions.h"

extern int yyparse();
extern int yylineno;

/*typedef struct t_class{
	char* name;
	struct t_method* constructor;
	struct VarDecl* parametres;
	struct t_method* methods;
	struct VarDecl* attributes;
	struct t_class* superClass;
	struct t_class* next;
}t_class;*/

/* REMPLISSAGE STRUCT DE CLASSE */
t_class* makeListClass(TreeP TreeClass){
	
	if(TreeClass != NIL(Tree)){
		t_class* myClass = NEW(1, t_class);
		
		/*LE NOM*/
		myClass->name = getChild(TreeClass, 0)->u.str;
		
		/*LA LISTE de PARAMETRES*/
		myClass->parametres = getChild(TreeClass, 2)->u.lvar;
		
		/*EXTENDS ?*/
		if(getChild(TreeClass, 3) == NIL(Tree)){
			myClass->superClass = myClass; /* On repasse par la liste de classes pour changer ce pointeur avec le bon.*/
		}else{
			myClass->superClass = NIL(t_class);
		}
		
	}else{
		return NIL(t_class);
	}
}

t_class* FindClass(t_class* listClass, char* str){
	
	if(0 == strcmp (listClass->name, str)){
			return listClass;
	}
	
	while(listClass->next != NIL(t_class)){
		listClass = listClass->next;
		if(0 == strcmp (listClass->name, str)){
			return listClass;
		}
	}
	return NIL(t_class);
}
/*
t_method* DMtoS(TreeP Tree){

	t_method* forest = NEW(1,t_method);
	if(Tree->op == DECL_METH){
		if(Tree->nbChildren == 3){

		}
		else{

		}
	}


}*/
