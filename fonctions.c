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

/* REMPLISSAGE DE LA LISTE DE CLASSES & OBJETS*/
list_ClassObjP makeListClassObj(TreeP TreeList){
	list_ClassObjP list = NIL(list_ClassObj);
	
	while(TreeList != NIL(Tree)){
		if(getChild(TreeList, 1)->op == CLAS){
			t_class* newClass = makeClass(getChild(getChild(TreeList, 1), 0), list->listClass);
			t_class* lastClass;
			
			if(list->listClass == NIL(t_class)){
				list->listClass = newClass;
				lastClass = newClass;
			}
			else{
				lastClass->next = newClass;
				lastClass = newClass;
			}
		}
		else if(getChild(TreeList, 1)->op == OBJ){
			t_object* newObj = makeObj(getChild(getChild(TreeList, 1), 0), list->listClass);
			t_object* lastObj;
			
			if(list->listObj == NIL(t_object)){
				list->listObj = newObj;
				lastObj = newObj;
			}
			else{
				lastObj->next = newObj;
				lastObj = newObj;
			}
		}
		TreeList = getChild(TreeList, 0);
	}
	return list;
}

/* REMPLISSAGE STRUCT DE CLASSE */
t_class* makeClass(TreeP TreeClass, t_class* firstClass){
	
	if(TreeClass != NIL(Tree)){
		t_class* myClass = NEW(1, t_class);
		
		/*LE NOM*/
		myClass->name = getChild(TreeClass, 0)->u.str;
		
		/*LA LISTE de PARAMETRES*/
		if(getChild(TreeClass, 1) != NIL(Tree)){
			myClass->parametres = getChild(TreeClass, 1)->u.lvar;
		}else{
			myClass->parametres = NIL(VarDecl);
		}
		
		/* CONSTRUCTOR */
		if(getChild(TreeClass, 3) != NIL(Tree)){
			myClass->constructor = makeConstructor(myClass, myClass->parametres, getChild(TreeClass, 3));
		}else{
			myClass->constructor = NIL(t_method);
		}
		
		/*EXTENDS ?*/
		if(getChild(TreeClass, 2) != NIL(Tree)){
			myClass->superClass = FindClass(firstClass, getChild(getChild(TreeClass, 2), 0)->u.str);
		}else{
			myClass->superClass = NIL(t_class);
		}
		
		/* LES METHODES  & LES ATTRIBUTS*/
		if(getChild(TreeClass, 4) != NIL(Tree)){
			myClass->methods = NIL(t_method);
			myClass->attributes = NIL(VarDecl);
		}else{
			myClass->methods = giveAllMethod(getChild(TreeClass, 4), firstClass);
			myClass->attributes = giveAllAttributes(getChild(TreeClass, 4), firstClass);
		}		
		
		return myClass;
		
	}else{
		return NIL(t_class);
	}
}

/* REMPLISSAGE STRUCT DE OBJECT */
t_object* makeObj(TreeP TreeObject, t_class* firstClass){
	
	if(TreeObject != NIL(Tree)){
		t_object* myObject = NEW(1, t_object);
		
		/*LE NOM*/
		myObject->name = getChild(TreeObject, 0)->u.str;
		
		/* LES METHODES  & LES ATTRIBUTS*/
		if(getChild(TreeObject, 1) != NIL(Tree)){
			myObject->methods = NIL(t_method);
			myObject->attributes = NIL(VarDecl);
		}else{
			myObject->methods = giveAllMethod(getChild(TreeObject, 1), firstClass);
			myObject->attributes = giveAllAttributes(getChild(TreeObject, 1), firstClass);
		}		
		
		return myObject;
		
	}else{
		return NIL(t_object);
	}
}

VarDeclP giveAllAttributes(TreeP tree, t_class* firstClass){
	VarDeclP list = NIL(VarDecl);
	
	while(tree != NIL(Tree)){
		if(getChild(tree, 0)->op == VAR_DEF_CHAMP){
			VarDeclP newChamp = getChild(getChild(tree, 0), 0)->u.lvar;
			VarDeclP last;
			
			if(list == NIL(VarDecl)){
				list = newChamp;
				last = newChamp;
			}
			else{
				last->next = newChamp;
				last = newChamp;
			}
		}
		tree = getChild(tree, 1);
	}
	return list;
}

t_method* giveAllMethod(TreeP tree, t_class* firstClass){
	t_method* list = NIL(t_method);
	
	while(tree != NIL(Tree)){
		if(getChild(tree, 0)->op == VAR_DEF_METH){
			t_method* newMeth = DMtoS(getChild(getChild(tree, 0), 0), firstClass);
			t_method* last;
			
			if(list == NIL(t_method)){
				list = newMeth;
				last = newMeth;
			}
			else{
				last->next = newMeth;
				last = newMeth;
			}
		}
		tree = getChild(tree, 1);
	}
	return list;
}

t_method* makeConstructor(t_class* class, VarDeclP param, TreeP corps){/* TODO */
	t_method* method = NEW(1,t_method);

	if(class != NIL(t_class)){
		/*NOM*/
		method->name = class->name;

		method->isRedef = FALSE; /*ici pas de redéfinition vu qu'il s'agit d'un constructeur*/

		/*PARAMETRES*/
		method->parametres = param;
		if(method->parametres != NIL(VarDecl))
		{
			method->nbParametres = 1;
			VarDeclP tmp = method->parametres;
			while(tmp->next != NIL(VarDecl)){
				tmp = tmp->next;
				method->nbParametres++;
			}
		}
		else{
			method->nbParametres = 0; 
		}

		method->returnType = class; /*TYPE DE RETOUR*/
		method->bloc = corps;		/*L'ensemble des instructions*/

			return method;
	}
	free(method);
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
			if(method->parametres != NIL(VarDecl))
			{
				method->nbParametres = 1;
				VarDeclP tmp = method->parametres;
				while(tmp->next != NIL(VarDecl)){
					tmp = tmp->next;
					method->nbParametres++;
				}
			}
			else{
				method->nbParametres = 0; 
			}

			/*TYPE DE RETOUR*/
			method->returnType = NEW(1,t_class);
			method->returnType = FindClass(listClass,getChild(Tree,2)->u.str);

			/*BLOC D'EXPRESSIONS*/
			method->bloc = getChild(Tree,5);

			return method;

			}
		else{			/*cas DeclMethod ::= Override DEF ID'(' ListParamClause ')' ClassClause IS block */

				/*OVERRIDE*/
				if(getChild(Tree,2) == NULL){
				method->isRedef = FALSE;	
				}
				else method->isRedef = TRUE;

				/*PARAMETRES*/
				method->parametres = getChild(Tree,4)->u.lvar;
				if(method->parametres != NIL(VarDecl))
				{
					method->nbParametres = 1;
					VarDeclP tmp = method->parametres;
					while(tmp->next != NIL(VarDecl)){
						tmp = tmp->next;
						method->nbParametres++;
					}
				}
				else{
				method->nbParametres = 0;

				}

			/*TYPE DE RETOUR*/ /*ici le l'option facultative de type de retour est a prendre en compte*/
			method->returnType = NEW(1,t_class);
			if(getChild(Tree,2)->u.str == NULL){
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
	free(method);
	return NULL;
}
