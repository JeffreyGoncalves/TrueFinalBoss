#include "fonctions.h"

extern int yyparse();
extern int yylineno;

/* REMPLISSAGE DE LA LISTE DE CLASSES & OBJETS*/
list_ClassObjP makeListClassObj(TreeP TreeList){
	list_ClassObjP list = NEW(1, list_ClassObj);
	
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
		myClass->name = getChild(TreeClass, 0)->u.lvar->name;
		
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
			t_class* classTempo = NEW(1, t_class);
			classTempo->name = getChild(getChild(TreeClass, 2), 0)->u.lvar->name; /* On lie la 'vraie' super-classe lors de la vérification contextuelle de portée.*/
			myClass->superClass = classTempo;

		}else{
			myClass->superClass = NIL(t_class);
		}
		
		/* LES METHODES  & LES ATTRIBUTS*/
		if(getChild(TreeClass, 4) == NIL(Tree)){
			myClass->methods = NIL(t_method);
			myClass->attributes = NIL(VarDecl);
		}else{
			myClass->methods = giveAllMethod(getChild(TreeClass, 4), firstClass);
			myClass->attributes = giveAllAttributes(getChild(TreeClass, 4), firstClass);
		}
		
		/** Ici, on regarde tous les paramètres possèdant un mot clef VAR afin de les ajouter dans la liste des attributs */
		VarDeclP tempo, VarBuffer = myClass->parametres;
		while(VarBuffer != NIL(VarDecl)){
			if(VarBuffer->coeur->res.i == 1){
				tempo = makeVarDeclP(VarBuffer->name, VarBuffer->coeur->_type->name,VarBuffer->coeur->value, 0);
				tempo->next = myClass->attributes;
				myClass->attributes = tempo;
			}
			VarBuffer = VarBuffer->next;
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
		myObject->name = getChild(TreeObject, 0)->u.lvar->name;
		
		/* LES METHODES  & LES ATTRIBUTS*/
		if(getChild(TreeObject, 1) == NIL(Tree)){
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
			VarDeclP newChamp = getChild(tree, 0)->u.lvar;
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
			
			t_method* newMeth = MakeMethod(getChild(getChild(tree, 0), 0), firstClass);
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

t_method* makeConstructor(t_class* class, VarDeclP param, TreeP corps){
	
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
	
	if(0 == strcmp(listClass->name, str)){
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

t_object* FindObject(t_object* listObj, char* str){
	
	if(0 == strcmp(listObj->name, str)){
			return listObj;
	}
	
	while(listObj->next != NIL(t_object)){
		listObj = listObj->next;
		if(0 == strcmp (listObj->name, str)){
			return listObj;
		}
	}
	return NIL(t_object);
}

t_method* MakeMethod(TreeP TreeM,t_class* listClass){
	
	t_method* method = NEW(1,t_method);
	
	if(TreeM->op == DECL_METH_1 || TreeM->op == DECL_METH_2){

		/*NOM DE LA METHODE*/
		method->name = getChild(TreeM,0)->u.lvar->name;
		if(TreeM->op == DECL_METH_1){					/*cas DeclMethod ::= Override DEF ID'(' ListParamClause ')' ':' ID AFF ExprRelop*/

			/*OVERRIDE*/
			if(getChild(TreeM,2) == NULL){
				method->isRedef = FALSE;	
			}
			else method->isRedef = TRUE;

			/*PARAMETRES*/
			if(getChild(TreeM,3) != NIL(Tree)){
				method->parametres = getChild(TreeM,3)->u.lvar;
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
			}else{
					method->parametres = NIL(VarDecl);
					method->nbParametres = 0;
			}
			
			/*TYPE DE RETOUR*/
			method->returnType = NEW(1,t_class);
			/*printf("%s\n",getChild(TreeM,1)->u.str);*/
			method->returnType->name = getChild(TreeM,1)->u.lvar->name;

			/*BLOC D'EXPRESSIONS*/
			method->bloc = getChild(TreeM,4);

			return method;

			}
		else{			/*cas DeclMethod ::= Override DEF ID'(' ListParamClause ')' ClassClause IS block */

				/*OVERRIDE*/
				if(getChild(TreeM,1) == NULL){
				method->isRedef = FALSE;	
				}
				else method->isRedef = TRUE;

				/*PARAMETRES*/
				if(getChild(TreeM,2) != NIL(Tree)){
					method->parametres = getChild(TreeM,2)->u.lvar;
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
				}else{
					method->parametres = NIL(VarDecl);
					method->nbParametres = 0;
				}

			/*TYPE DE RETOUR*/ /*ici le l'option facultative de type de retour est a prendre en compte*/
			method->returnType = NEW(1,t_class);
			if(getChild(TreeM,3) == NIL(Tree)){
				method->returnType->name = "Void"; 		/* On lie la 'vraie' classe lors de la vérification contextuelle de portée.*/
			}
			else{
				method->returnType->name = getChild(TreeM,3)->u.lvar->name;
			}
			
			/*BLOC*/
			method->bloc = getChild(TreeM,4);

			return method;
		}
	}
	free(method);
	return NULL;
}

void afficheClass(t_class* liste){
	
	printf("*****************************	LISTE DES CLASSES	*****************************\n\n");
	
	while(liste != NIL(t_class)){
		printf("***	NAME : %s\n", liste->name);
		printf("***	PARAMETRES : "); afficheParam(liste->parametres);
		printf("\n");
		printf("***	SUPER-CLASSE : ");
		if(liste->superClass != NIL(t_class)){printf("%s\n",liste->superClass->name);}
		else{printf("\n");}
		printf("***	CONSTRUCTEUR : %d\n", liste->constructor != NIL(t_method));
		printf("***	ATTRIBUTS : "); afficheParam(liste->attributes);
		printf("\n");
		printf("***	METHODES :\n"); afficheNomMethod(liste->methods);
		printf("-------------------------\n");
		liste = liste->next;
	}
}

void afficheObj(t_object* liste){
	
	printf("*****************************	LISTE DES OBJETS	*****************************\n\n");
	
	while(liste != NIL(t_object)){
		printf("***	NAME : %s\n", liste->name);
		printf("***	ATTRIBUTS : "); afficheParam(liste->attributes);
		printf("\n");
		printf("***	METHODES :\n"); afficheNomMethod(liste->methods);
		printf("-------------------------\n");
		liste = liste->next;
	}
}

void afficheParam(VarDeclP liste){

	while(liste != NIL(VarDecl)){
		printf("%s : %s (isInit : %d), ", liste->name, liste->coeur->_type->name, liste->isInit);
		liste = liste->next;
	}
	

}

void afficheNomMethod(t_method* liste){

	while(liste != NIL(t_method)){
		printf("   %s(", liste->name);
		afficheParam(liste->parametres);
		printf(")	return %s\n", liste->returnType->name);
		liste = liste->next;
	}
	printf("\n");

}

void compile(TreeP listClassObject, TreeP core){
	
	/* MISE EN PLACE DES STRUCTURES */
	list_ClassObjP environnement = makeListClassObj(listClassObject);
	creationClasseIntegerStringVoid(environnement);
	afficheClass(environnement->listClass);
	afficheObj(environnement->listObj);
	
	/** VERIFICATION CONTEXTUELLE */
	printf("********	Debut de vérification contextuelle	********\n");
	
	printf("\n\n********	Mise en place des liens SuperClass			********\n");
	if(!verificationSuperClass(environnement))
			abort();
			
	printf("\n\n********			Portée			********\n");
	verifPorteeProg(core, environnement);
	
	printf("\n\n********			Typage			********\n");
	/*if(!verificationTypage(environnement, core))
		abort();*/
		
	printf("\n\n********			Paramètre			********\n");

}

void creationClasseIntegerStringVoid(list_ClassObjP env){	
	
	t_class* _void = NEW(1, t_class);
	t_class* integer = NEW(1, t_class);
	t_class* string = NEW(1, t_class);
	
	/**	INTEGER */
	/*nom*/
	integer->name = "Integer";
	/*constructeur*/
	integer->constructor = NEW(1, t_method);
		integer->constructor->name = "Integer";
		integer->constructor->isRedef = 0;
		integer->constructor->bloc = NIL(Tree);
		integer->constructor->nbParametres = 0;
		integer->constructor->parametres = NIL(VarDecl);
		integer->constructor->returnType = integer;
	/*parametre*/
	integer->parametres = NIL(VarDecl);
	/*attribut*/
	/*integer->attributes = NEW(1, VarDecl);*/
	/*methode*/
	integer->methods = NEW(1, t_method);
		integer->methods->name = "toString";
		integer->methods->isRedef = 0;
		integer->methods->bloc = NIL(Tree);
		integer->methods->nbParametres = 0;
		integer->methods->parametres = NIL(VarDecl);
		integer->methods->returnType = string;
	/* next et superClass sont deja sur NIL(t_class)*/
	
	
	/**	VOID */
	/*nom*/
	_void->name = "Void";
	/*constructeur*/
	_void->constructor = NIL(t_method);
	/*parametre*/
	_void->parametres = NIL(VarDecl);
	/*attribut*/
	_void->attributes = NIL(VarDecl);
	/*methode*/
	_void->methods = NIL(t_method);
	/* next et superClass sont deja sur NIL(t_class)*/
	
	
	/**	STRING */
	/*nom*/
	string->name = "String";
	/*constructeur*/
	string->constructor = NEW(1, t_method);
		string->constructor->name = "String";
		string->constructor->isRedef = 0;
		string->constructor->bloc = NIL(Tree);
		string->constructor->nbParametres = 0;
		string->constructor->parametres = NIL(VarDecl);
		string->constructor->returnType = string;
	/*parametre*/
	string->parametres = NIL(VarDecl);
	/*attribut*/
	/*string->attributes = NEW(1, VarDecl);*/
	/*string->attributes->coeur = NEW(1, t_variable);*/
	/*methode*/
	string->methods = NEW(1, t_method);
		string->methods->name = "print";
		string->methods->isRedef = 0;
		string->methods->bloc = NIL(Tree);
		string->methods->nbParametres = 0;
		string->methods->parametres = NIL(VarDecl);
		string->methods->returnType = _void;
	string->methods->next = NEW(1, t_method);
		string->methods->next->name = "println";
		string->methods->next->isRedef = 0;
		string->methods->next->bloc = NIL(Tree);
		string->methods->next->nbParametres = 0;
		string->methods->next->parametres = NIL(VarDecl);
		string->methods->next->returnType = _void;
	/* next et superClass sont deja sur NIL(t_class)*/
	
	/* On les lie pour faire la chaine de classe*/
	integer->next = string;
	string->next = _void;
	
	t_class* iterator = env->listClass;
	if(iterator == NIL(t_class)) env->listClass = integer;
	else{
		while(iterator->next != NIL(t_class)){
			iterator = iterator->next;
		}
		iterator->next = integer;
	}
}
