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
t_class* makeListClass(TreeP TreeClass, t_class* firstClass){
	
	if(TreeClass != NIL(Tree)){
		t_class* myClass = NEW(1, t_class);
		
		/*LE NOM*/
		myClass->name = getChild(TreeClass, 0)->u.str;
		
		/*LA LISTE de PARAMETRES*/
		if(getChild(TreeClass, 4) != NIL(Tree)){
			myClass->parametres = getChild(TreeClass, 2)->u.lvar;
			myClass->constructor = makeConstructor(myClass, myClass->parametres, getChild(TreeClass, 4));
		}else{
			myClass->parametres = NIL(VarDecl);
			myClass->constructor = NIL(t_method);
		}
		
		/*EXTENDS ?*/
		if(getChild(TreeClass, 3) != NIL(Tree)){
			myClass->superClass = FindClass(firstClass, getChild(getChild(TreeClass, 3), 1)->u.str);
		}else{
			myClass->superClass = NIL(t_class);
		}
		
		/* LES METHODES  & LES ATTRIBUTS*/
		if(getChild(TreeClass, 5) != NIL(Tree)){
			myClass->methods = NIL(t_method);
			myClass->attributes = NIL(VarDecl);
		}else{
			myClass->methods = giveAllMethod(getChild(TreeClass, 5), firstClass);
			myClass->attributes = giveAllAttributes(getChild(TreeClass, 5), firstClass);
		}
		
				
		
		return myClass;
		
	}else{
		return NIL(t_class);
	}
}

VarDeclP giveAllAttributes(TreeP tree, t_class* firstClass){
	
}

t_method* giveAllMethod(TreeP tree, t_class* firstClass){
	t_method* list = NIL(t_method);
	
	while(tree != NIL(Tree)){
		if(getChild(tree, 1)->op == VAR_DEF_METH){
			t_method* newMeth = DMtoS(getChild(getChild(tree, 1), 1), firstClass);
			
			if(list == NIL(t_method)){newMeth = list;}
			else{
				list->next = newMeth;
			}
		}
		tree = getChild(tree, 2);
	}
	return list;
}

t_method* makeConstructor(t_class* class, VarDeclP param, TreeP corps){/* TODO */
	return NULL;
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


t_method* DMtoS(TreeP Tree,t_class* listClass){

	t_method* method = NEW(1,t_method);
	if(Tree->op == DECL_METH){

		/*NOM DE LA METHODE*/
		method->name = getChild(Tree,1)->u.str;
		if(Tree->nbChildren == 3){					/*cas DeclMethod ::= Override DEF ID'(' ListParamClause ')' ':' ID AFF ExprRelop*/

			/*OVERRIDE*/
			if(getChild(Tree,3) == NULL){
				method->isRedef = FALSE;	
			}
			else method->isRedef = TRUE;

			/*PARAMETRES*/
			method->parametres = getChild(Tree,4)->u.lvar;
			method->nbParametres = sizeof(method->parametres)/sizeof(VarDecl);

			/*TYPE DE RETOUR*/
			method->returnType = NEW(1,t_class);
			method->returnType = FindClass(listClass,getChild(Tree,2)->u.str);

			/*BLOC D'EXPRESSIONS*/
			method->bloc = getChild(Tree,5);

			return method;

		}
		else{

			/*OVERRIDE*/
			if(getChild(Tree,2) == NULL){
				method->isRedef = FALSE;	
			}
			else method->isRedef = TRUE;

			/*PARAMETRES*/
			method->parametres = getChild(Tree,3)->u.lvar;
			method->nbParametres = sizeof(method->parametres)/sizeof(VarDecl);

			/*TYPE DE RETOUR*/ /*ici le l'option facultative de type de retour est a prendre en compte*/
			method->returnType = NEW(1,t_class);
			if(getChild(Tree,4)->u.str == NULL){
				method->returnType = FindClass(listClass,"void");
			}
			else{
				method->returnType = FindClass(listClass,getChild(Tree,4)->u.str);
			}

			/*BLOC*/
			method->bloc = getChild(Tree,4);

			return method;
		}
	}
	return NULL;
}
