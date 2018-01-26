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
		if(0 == strcmp(listClass->name, str)){
			return listClass;
		}
	}
	return NIL(t_class);
}

t_method* DMtoS(TreeP Tree){

	t_method* method = NEW(1,t_method);
	if(Tree->op == DECL_METH){

		/*NOM DE LA METHODE*/
		method->name = getChild(Tree,1)->u.str;
		if(Tree->nbChildren == 3){					/*cas DeclMethod ::= Override DEF ID'(' ListParamClause ')' ':' ID AFF ExprRelop*/

			/*OVERRIDE*/
			if(getChild(Tree,3) == NULL){
				method->isReDef == FALSE;	
			}
			else{method->isReDef == TRUE;}

			/*PARAMETRES*/
			method->parametres = getChild(Tree,4)->u.lvar;
			method->nbParametres = sizeof(method->parametres)/sizeof(VarDecl);

			/*TYPE DE RETOUR*/
			method->returnType = NEW(1,t_class);
			method->returnType->name = getChild(Tree,2)->u.str;

			/*EXPRESSIONS*/
			method->instructions = getChild(Tree,5);

			return method;

		}
		else{

			/*OVERRIDE*/
			if(getChild(Tree,2) == NULL){
				method->isReDef == FALSE;	
			}
			else{method->isReDef == TRUE;}

			/*PARAMETRES*/
			method->parametres = getChild(Tree,3)->u.lvar;
			method->nbParametres = sizeof(method->parametres)/sizeof(VarDecl);

			/*TYPE DE RETOUR*/
			method->returnType = NEW(1,t_class);

			/*EXPRESSIONS*/
			method->instructions = getChild(Tree,4);

			return method;
		}
	}
	else{
		return NULL;
	}

}
