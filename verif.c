#include "verif.h"

extern char *strdup(const char*);

extern void setError(int code);

/***************** Verifications contextuelles liees a la portee *****************/
/*NB : FAUDRAIT-IL VERIFIER UNIQUEMENT LA PORTEE SI ELLE EST FAUSSE OU LA PORTEE ET LE TYPE MEME SI LE PREMIER SE REVELE FAUX ? */
void verifPorteeProg(TreeP tree, list_ClassObjP classObjList)
{
	if(tree != NIL(Tree))
	{
		/* Verification de portee a faire :
		 * --> utilisation de variables dans un bloc n'existant pas
		 * --> declaration de variables multiples ayant le meme nom dans un bloc (partie declaration uniquement)
		 * --> declaration de champs multiples ayant le meme nom dans un objet/classe
		 * --> utilisation de variables dans une methode n'existant pas en tant que champ */
		
		
		/* Si l'operateur de notre arbre est un bloc, alors on effectue la verification
		 * pour toutes les instructions du bloc en prenant en compte les declarations 
		 * preemptives, y compris si elles n'existent pas. Il faut aussi traiter les 
		 * blocs dans le bloc vu qu'un bloc est une instruction. 
		 */
		 
		 /* TreeP tree : représente le corps du programme.
		  * list_ClassObjP classObjList : représente l'ensemble des déclarations des objets et classes.
		  */
		  
		/* Verification partie classe/objet puis
		 * Verification du bloc principal (portee) */
		bool isCorrect = verifPorteeClassObj(classObjList)
		  /*&& verifPorteeBloc(tree, NULL, classObjList->listObj)*/;
		 
		printf("[[[[[[[[[[[[[[[[[[[L'analyse de portée est donc %d]]]]]]]]]]]]]]]]]]]\n",isCorrect);
		
		/*A DISCUTER */ 
		if(!isCorrect)
			abort();
	}
}

bool verifPorteeMeth(TreeP tree, t_class *class)
{
	return FALSE;
}

bool verifPorteeInst(TreeP inst, VarDeclP listDecl, list_ClassObjP classObjList)
{
	bool toReturn;
	printf("ici\n");
		/* Bloc */
		if(inst->op == I_BLOC) toReturn = verifPorteeBloc(inst, listDecl, classObjList);
			
		/* Return, non autorise ici s'il n'existe pas de VarDecl avec le nom "result" dans la liste. */
		else if(inst->op == I_RETURN)
		{
			printf("ici\n");
			VarDeclP varSel = listDecl;
			while(varSel != NULL)
			{
				if(!strcmp(varSel->name, "result"))
				{
					free(inst->u.lvar);
					inst->u.lvar = varSel;
				}
				varSel = varSel->next;
			}
			if(!strcmp(inst->u.lvar->name, "result_tempo")){
				setError(RETURN_ERROR);
				toReturn = FALSE;
			}
		}
		
		/* ITE */
		else if(inst->op == I_ITE)
		{
			printf("ici\n");
			TreeP Expr = getChild(inst, 0),
				  Inst1 = getChild(inst, 1),
				  Inst2 = getChild(inst, 2);
			toReturn = verifPorteeExpr(Expr, listDecl, classObjList)
					&& verifPorteeInst(Inst1, listDecl, classObjList) 
					&& verifPorteeInst(Inst2, listDecl, classObjList);
		}
		
		/* Affectation */
		else if(inst->op == I_AFF)
		{
			printf("ici\n");
			TreeP Obj = getChild(inst, 0),
				  Expr = getChild(inst, 1);
			toReturn = verifPorteeExpr(Obj, listDecl, classObjList)
					&& verifPorteeExpr(Expr, listDecl, classObjList);
		}
		
		/* Expression */
		else if(inst->op == I_EXPRRELOP)
		{
			printf("ici\n");
			toReturn = verifPorteeExpr(getChild(inst, 0), listDecl, classObjList);
		}  
		
		else
		{
			printf("ici\n");
			toReturn = FALSE;
			/* CAS NE DEVANT JAMAIS ARRIVER */
		}

	return toReturn;
}

bool verifPorteeClassObj(list_ClassObjP classObjList)
{	
	return verifPorteeListClass(classObjList)
		&& verifPorteeListObject(classObjList);
}

bool verifPorteeListObject(list_ClassObjP classObjList)
{
	printf("********		Portée des objets		********\n");
	bool toReturn = TRUE;
	t_object* objectBuffer = classObjList->listObj;
	while(objectBuffer != NIL(t_object)){
		if(!verifPorteeObject(objectBuffer, classObjList) && toReturn) toReturn = FALSE;
		objectBuffer = objectBuffer->next;
	}
	printf("*****************************************\n");
	return toReturn;
}

bool verifPorteeObject(t_object* object, list_ClassObjP classObjList)
{
	bool toReturn = TRUE;
	printf("|||||||%s:\n",object->name);
	/*	On vérifie que le nom n'existe pas deja */
	toReturn = toReturn && verificationNomClasse(classObjList, object->name);
	printf("%d Nom\n",toReturn);
	
	/* Verification attributs */
	VarDeclP VarDeclBuffer = object->attributes;
	while(VarDeclBuffer != NIL(VarDecl)){
		if(!verificationNomVarDecl(object->attributes, VarDeclBuffer->name) && toReturn) toReturn = FALSE;
		
		t_class* ClassBuffer = FindClass(classObjList->listClass, VarDeclBuffer->coeur->_type->name);
		if(ClassBuffer == NIL(t_class)) 
		{
			setError(CLASS_NOT_FOUND);
			toReturn = FALSE;
		}
		free(VarDeclBuffer->coeur->_type); /** C'était une classe temporaire.*/
		VarDeclBuffer->coeur->_type = ClassBuffer;
		
		VarDeclBuffer = VarDeclBuffer->next;
	}
	printf("%d Attributs\n",toReturn);
	
	/* Verification methodes */
	t_method* methodBuffer = object->methods;
	while(methodBuffer != NIL(t_method))
	{
		printf("	Portée de [%s]:\n",methodBuffer->name);
		if((!verificationNomMethod(object->methods, methodBuffer->name) || !verifPorteeMethodO(methodBuffer, object, classObjList))
		 && toReturn) 
		{
			 toReturn = FALSE;
		}
		methodBuffer = methodBuffer->next;
	}
	printf("%d Methodes\n",toReturn);
	printf("________________________________________\n");
	return toReturn;
}

bool verifPorteeListClass(list_ClassObjP classObjList)
{
	printf("********		Portée des classes		********\n");
	bool toReturn = TRUE;
	t_class* iterator = classObjList->listClass;
	
	while(iterator != NIL(t_class))
	{
		if(!verifPorteeClass(iterator, classObjList) && toReturn) toReturn = FALSE;
		iterator = iterator->next;
	}
	
	return toReturn;
}

bool verifPorteeClass(t_class* class, list_ClassObjP classObjList){
	
	bool toReturn = TRUE;
	t_class* ClassBuffer;
	
	
	printf("|||||||%s:\n",class->name);
	/*	On vérifie que le nom n'existe pas deja */
	if(!verificationNomClasse(classObjList, class->name) && toReturn) toReturn = FALSE;
	printf("%d Nom\n",toReturn);
	/*	Verification de la super-classe */
	if(class->superClass != NIL(t_class)){
		ClassBuffer = FindClass(classObjList->listClass, class->superClass->name);
		if(ClassBuffer == NIL(t_class)) 
		{
			setError(CLASS_NOT_FOUND);
			toReturn = FALSE;
		}
		free(class->superClass); /** C'était une classe temporaire.*/
		class->superClass = ClassBuffer;
	}
	printf("%d Super-classe\n",toReturn);
	/* Verification parametres */
	VarDeclP VarDeclBuffer = class->parametres;
	while(VarDeclBuffer != NIL(VarDecl)){
		if(!verificationNomVarDecl(class->parametres, VarDeclBuffer->name) && toReturn) toReturn = FALSE;
		
		ClassBuffer = FindClass(classObjList->listClass, VarDeclBuffer->coeur->_type->name);
		if(ClassBuffer == NIL(t_class) && toReturn) toReturn = FALSE;
		free(VarDeclBuffer->coeur->_type); /** C'était une classe temporaire.*/
		VarDeclBuffer->coeur->_type = ClassBuffer;
		
		VarDeclBuffer = VarDeclBuffer->next;
	}
	printf("%d Parametres\n",toReturn);
	/* Verification attributs */
	VarDeclBuffer = class->attributes;
	while(VarDeclBuffer != NIL(VarDecl)){
		if(!verificationNomVarDecl(class->attributes, VarDeclBuffer->name) && toReturn) toReturn = FALSE;
		
		ClassBuffer = FindClass(classObjList->listClass, VarDeclBuffer->coeur->_type->name);
		if(ClassBuffer == NIL(t_class))
		{
			setError(CLASS_NOT_FOUND);
			toReturn = FALSE;
		}
		free(VarDeclBuffer->coeur->_type); /** C'était une classe temporaire.*/
		VarDeclBuffer->coeur->_type = ClassBuffer;
		
		VarDeclBuffer = VarDeclBuffer->next;
	}
	printf("%d Attributs\n",toReturn);
	/* Verification methodes */
	t_method* methodBuffer = class->methods;
	while(methodBuffer != NIL(t_method)){
		printf("	Portée de [%s]:\n",methodBuffer->name);
		
		if(!verificationNomMethod(class->methods, methodBuffer->name) && toReturn) toReturn = FALSE;
		printf("		%d nom\n",toReturn);
		if(!verifPorteeMethodC(methodBuffer, class, classObjList) && toReturn) toReturn = FALSE;
		
		methodBuffer = methodBuffer->next;
	}
	printf("%d Methodes\n",toReturn);
	/* Verification constructeur */
	methodBuffer = class->constructor;
	if(methodBuffer != NIL(t_method)) {
		if(!verifPorteeConstructor(methodBuffer, class, classObjList) && toReturn) toReturn = FALSE;
		printf("%d Constructeurs\n",toReturn);
	}
	printf("________________________________________\n");
	return toReturn;
}

bool verifPorteeMethodC(t_method* method, t_class* class, list_ClassObjP classObjList)
{
	bool toReturn = TRUE;
	t_class* ClassBuffer;
	
	
	/* Verification parametres */
	VarDeclP VarDeclBuffer = method->parametres;
	while(VarDeclBuffer != NIL(VarDecl)){
		if(!verificationNomVarDecl(method->parametres, VarDeclBuffer->name) && toReturn) toReturn = FALSE;
		
		ClassBuffer = FindClass(classObjList->listClass, VarDeclBuffer->coeur->_type->name);
		if(ClassBuffer == NIL(t_class))
		{
			setError(CLASS_NOT_FOUND);
			toReturn = FALSE;
		}
		free(VarDeclBuffer->coeur->_type); /** C'était une classe temporaire.*/
		VarDeclBuffer->coeur->_type = ClassBuffer;
		
		VarDeclBuffer = VarDeclBuffer->next;
	}
	printf("		%d paramètres\n",toReturn);
	
	/* Verification du type de retour */
	ClassBuffer = FindClass(classObjList->listClass, method->returnType->name);
	if(ClassBuffer == NIL(t_class))
	{
		setError(CLASS_NOT_FOUND);
		toReturn = FALSE;
	}
	/*free(method->returnType); *//** C'était une classe temporaire.*/
	method->returnType = ClassBuffer;
	printf("		%d type du return\n",toReturn);
	
	
	if(!verifPorteeBloc(method->bloc, InitialisationSuperThisResultC(method, class, method->parametres), classObjList)) toReturn = FALSE;
	printf("		%d bloc ok\n",toReturn);
	
	return toReturn;
}

bool verifPorteeMethodO(t_method* method, t_object* object, list_ClassObjP classObjList){
	bool toReturn = TRUE;
	t_class* ClassBuffer;
	
	/* Verification parametres */
	VarDeclP VarDeclBuffer = method->parametres;
	while(VarDeclBuffer != NIL(VarDecl)){
		if(!verificationNomVarDecl(method->parametres, VarDeclBuffer->name) && toReturn) toReturn = FALSE;
		
		ClassBuffer = FindClass(classObjList->listClass, VarDeclBuffer->coeur->_type->name);
		if(ClassBuffer == NIL(t_class))
		{
			setError(CLASS_NOT_FOUND);
			toReturn = FALSE;
		}
		free(VarDeclBuffer->coeur->_type); /** C'était une classe temporaire.*/
		VarDeclBuffer->coeur->_type = ClassBuffer;
		
		VarDeclBuffer = VarDeclBuffer->next;
	}
	printf("		%d paramètres\n",toReturn);
	
	/* Verification du type de retour */
	ClassBuffer = FindClass(classObjList->listClass, method->returnType->name);
	if(ClassBuffer == NIL(t_class))
	{
		setError(CLASS_NOT_FOUND);
		toReturn = FALSE;
	}
	/*free(method->returnType);*/ /** C'était une classe temporaire.*/
	method->returnType = ClassBuffer;
	printf("		%d type du return\n",toReturn);
	
	if(!verifPorteeBloc(method->bloc, InitialisationSuperThisResultO(method, object, method->parametres), classObjList)) toReturn = FALSE;
	printf("		%d bloc ok\n",toReturn);
	
	return toReturn;
}

bool verifPorteeConstructor(t_method* method, t_class* class, list_ClassObjP classObjList){
	
	printf("	Portée de [%s]:\n",method->name);
	
	/* PEU de chose à vérifier ici. */
	bool toReturn = TRUE;
	
	/* Verification parametres */
	VarDeclP VarDeclBuffer = method->parametres;
	while(VarDeclBuffer != NIL(VarDecl)){
		if(!verificationNomVarDecl(method->parametres, VarDeclBuffer->name) && toReturn) toReturn = FALSE;
		
		t_class* ClassBuffer = FindClass(classObjList->listClass, VarDeclBuffer->coeur->_type->name);
		if(ClassBuffer == NIL(t_class))
		{
			setError(CLASS_NOT_FOUND);
			toReturn = FALSE;
		}
		/*free(VarDeclBuffer->coeur->_type);*//** C'était une classe temporaire.*/
		VarDeclBuffer->coeur->_type = ClassBuffer;
		
		VarDeclBuffer = VarDeclBuffer->next;
	}
	printf("		paramètres ok\n");
	
	/* le type de retour est déjà vérifié de par la génération des structures. */
	
	/*  Idem pour le nom. */
	
	if(!verifPorteeBloc(method->bloc, InitialisationSuperThisResultC(method, class, method->parametres), classObjList)) toReturn = FALSE;
	printf("		bloc ok\n");
	
	return toReturn;
}

bool verifPorteeBloc(TreeP tree, VarDeclP listDecl, list_ClassObjP classObjList)
{	
	printf("ici\n");
	bool toReturn = TRUE;
	VarDeclP listVarDecl = getChild(tree, 0)->u.lvar;
	TreeP listInst = getChild(tree,1);
	
	/**** Verification de la liste de declaration ****/
	VarDeclP iterator = listVarDecl;
	while(iterator != NIL(VarDecl)){
		/* On regarde si la variable n'a pas déjà été déclarée dans ce bloc, puis si le nom super/result/this est utilisé. */
		if(!verificationNomVarDecl(listVarDecl, iterator->name)){
			setError(REDECL_ERROR);
			toReturn = FALSE;
		}else if(!strcmp(iterator->name, "this") || !strcmp(iterator->name, "super") || !strcmp(iterator->name, "result")){
			setError(RESERV_DECL_ERROR);
			toReturn = FALSE;
		}
		
		/* On associe les "vraies" classes pour les variables. */
		t_class* ClassBuffer = FindClass(classObjList->listClass, iterator->coeur->_type->name);
		if(ClassBuffer == NIL(t_class))
		{
			setError(CLASS_NOT_FOUND);
			toReturn = FALSE;
		}
		free(iterator->coeur->_type); /** C'était une classe temporaire.*/
		iterator->coeur->_type = ClassBuffer;
		
		iterator = iterator->next;
	}
	printf("ici\n");
	/*************************************************/
	
	/****	Ajout des declarations precedentes non prioritaires	******/
	/* Note : cela permet de gerer le masquage (les variables 
	 * du bloc sont prioritaires sur les precedentes */
	if(listVarDecl != NIL(VarDecl))
	{
		/*On va chercher le dernier élément de la liste prioritaire*/
		iterator = listVarDecl;
		while(iterator->next != NIL(VarDecl)) iterator = iterator->next;
		iterator->next = listDecl;
		listDecl = listVarDecl;
	}
	/*****************************************************************/
	printf("ici\n");
	/*** Verification des instructions (portee) ***/
	if(listInst != NULL)
	{
		while(listInst->nbChildren == 2)
		{
			toReturn = toReturn && verifPorteeInst(getChild(listInst, 0), listVarDecl, classObjList);
			listInst = getChild(listInst, 1);
		}
		toReturn = toReturn && verifPorteeInst(getChild(listInst, 0), listVarDecl, classObjList);
	}
	/**********************************************/
	printf("ici\n");
	return toReturn;
}

/* A TRAVAILLER */
bool verifPorteeExpr(TreeP Expr, VarDeclP listDecl, list_ClassObjP classObjList)
{
	bool toReturn = TRUE;
	int i; /* Sert de variable de parcours */
	
	/* Expression feuille */
	if(Expr->nbChildren == 0)
	{
		/*ID (les variables crees plus recemment sont prioritaires) */
		if(Expr->op == _ID)
		{
			/* Variables Crees dans le bloc */
			VarDeclP varSel = listDecl;
			while(varSel != NULL)
			{
				if(!strcmp(varSel->name, Expr->u.lvar->name))
				{
					free(Expr->u.lvar);
					Expr->u.lvar = varSel;
				}
				varSel = varSel->next;
			}
			
			/* Objets isoles */
			t_object *objSel = classObjList->listObj;
			while(objSel != NULL)
			{
				if(!strcmp(objSel->name, Expr->u.lvar->name))
				{
					Expr->u.lvar->coeur->_obj = objSel;
				}
				objSel = objSel->next;
			}
			
			/* Aucun objet n'a ete trouve */
			setError(VAR_NOT_FOUND);
			toReturn = FALSE;
		}
		else return FALSE;
	}
	
	else
	{
		t_class* classBuffer;
		t_object* objectBuffer;
		bool estTrouver = FALSE;
		
		switch(Expr->op){
			case INST:
				toReturn = toReturn && verifPorteeExpr(getChild(Expr, 1), listDecl, classObjList);
				classBuffer = FindClass(classObjList->listClass, getChild(Expr, 1)->u.lvar->name);
				if(classBuffer == NIL(t_class)){
					setError(CLASS_NOT_FOUND);
					toReturn = FALSE;
				}
				else{
					getChild(Expr, 0)->u.lvar->coeur->_type = classBuffer;
				}
				break;
				
			case E_SELECT:
				/* On cherche le ID_C, s'il existe ou non.*/
				if(getChild(Expr, 0)->op == _ID && getChild(Expr, 0)->u.lvar->name[0] <= 90){
					objectBuffer = FindObject(classObjList->listObj, getChild(Expr, 1)->u.lvar->name);
					
					if(objectBuffer == NIL(t_object)){
						setError(CLASS_NOT_FOUND);
						toReturn = FALSE;
					}
					else{
						getChild(Expr, 0)->u.lvar->coeur->_obj = objectBuffer;
					}
				}else{/**		cas avec Object.ID ou (ExprRelop).ID		*/
					toReturn = toReturn && verifPorteeExpr(getChild(Expr, 0), listDecl, classObjList);
					classBuffer = verifcationTypageNoeud(getChild(Expr, 0), classObjList).type.class;
					if(classBuffer == NIL(t_class)){
						setError(CLASS_NOT_FOUND);
						toReturn = FALSE;
					}
				}
				
				/* On cherche si l'attribut exite bel et bien dans la classe/objet trouvé. */
				VarDeclP varBuffer;
				
				if(classBuffer != NIL(t_class)){
					varBuffer = classBuffer->attributes;
					
					while(varBuffer != NIL(VarDecl)){
						if (!strcmp(varBuffer->name, getChild(Expr, 1)->u.lvar->name)) estTrouver = TRUE;
						varBuffer = varBuffer->next;
					}
				}else if(objectBuffer != NIL(t_object)){
					varBuffer = objectBuffer->attributes;

					while(varBuffer != NIL(VarDecl)){
						if (!strcmp(varBuffer->name, getChild(Expr, 1)->u.lvar->name)) estTrouver = TRUE;
						varBuffer = varBuffer->next;
					}
				}
				
				if(estTrouver == TRUE){
					getChild(Expr, 1)->u.lvar->coeur->_obj = objectBuffer;
					getChild(Expr, 1)->u.lvar->coeur->_type = classBuffer;
				}else{
					setError(VAR_NOT_FOUND);
					toReturn = FALSE;
				}				
				break;
				
			case E_CALL_METHOD:
				/* On cherche le ID_C, s'il existe ou non.*/
				if(getChild(Expr, 0)->op == _ID && getChild(Expr, 0)->u.lvar->name[0] <= 90){
					objectBuffer = FindObject(classObjList->listObj, getChild(Expr, 1)->u.lvar->name);
					
					if(objectBuffer == NIL(t_object)){
						setError(CLASS_NOT_FOUND);
						toReturn = FALSE;
					}
					else{
						getChild(Expr, 0)->u.lvar->coeur->_obj = objectBuffer;
					}
				}else{/**		cas avec Object.ID ou (ExprRelop).ID		*/
					toReturn = toReturn && verifPorteeExpr(getChild(Expr, 0), listDecl, classObjList);
					classBuffer = verifcationTypageNoeud(getChild(Expr, 0), classObjList).type.class;
					if(classBuffer == NIL(t_class)){
						setError(CLASS_NOT_FOUND);
						toReturn = FALSE;
					}
				}
				
				/* On cherche si la methode exite bel et bien dans la classe/objet trouvé. */
				t_method* methodBuffer;
				
				if(classBuffer != NIL(t_class)){
					methodBuffer = classBuffer->methods;
					
					while(methodBuffer != NIL(t_method)){
						if (!strcmp(methodBuffer->name, getChild(Expr, 1)->u.lvar->name)) estTrouver = TRUE;
						methodBuffer = methodBuffer->next;
					}
				}else if(objectBuffer != NIL(t_object)){
					methodBuffer = objectBuffer->methods;

					while(methodBuffer != NIL(t_method)){
						if (!strcmp(methodBuffer->name, getChild(Expr, 1)->u.lvar->name)) estTrouver = TRUE;
						methodBuffer = methodBuffer->next;
					}
				}
				
				if(estTrouver == TRUE){
					getChild(Expr, 1)->u.lvar->coeur->_obj = objectBuffer;
					getChild(Expr, 1)->u.lvar->coeur->_type = classBuffer;
				}else{
					setError(VAR_NOT_FOUND);
					toReturn = FALSE;
				}				
				break;
				
			default:
				/*bool toReturn = TRUE;*/
				for(i=0;i<Expr->nbChildren;i++)
					toReturn = toReturn && verifPorteeExpr(getChild(Expr, i), listDecl, classObjList);
	
		}
	}
	
	return toReturn;
}
/******************************************************************************************/

Vtypage verifcationTypageNoeud(TreeP noeud, list_ClassObjP env){
	
	Vtypage result;
	Vtypage veriFils[5];
	int i;
	t_method* m;
	result.succes = 1;
	
	if(noeud == NIL(Tree)) return result;
	
	for(i=0 ; i<noeud->nbChildren ; i++){
		veriFils[i] = verifcationTypageNoeud(getChild(noeud, i), env);
		if(!(veriFils[i].succes)){
			result.succes = 0;
			return result;
		}
	}
	
	switch(noeud->op){
		
		case I_BLOC:
			return result;
			/*return verifTypageSuccesFils(noeud->nbChildren, noeud, env);*/
		
		case DECL:
			return verifcationTypageListVarDecl(noeud->u.lvar, env);
			
		case LIST_PARAM:
			return verifcationTypageListVarDecl(noeud->u.lvar, env);
			
		case LIST_INST:
			return result;
			/*return verifTypageSuccesFils(noeud->nbChildren, noeud, env);*/

		case I_RETURN: /* PAS FINI ! */
			return result;
		
		case I_EXPRRELOP:
			return result;
			/*return verifTypageSuccesFils(noeud->nbChildren, noeud, env);*/
			
		case I_ITE:
			if(0 == strcmp(veriFils[0].type.class->name,"Interger")){
				return result;
			}
			break;
			
		case I_AFF:
			if(0 == strcmp(veriFils[0].type.class->name, veriFils[1].type.class->name)){
				return result;
			}
			break;
			
		case SUM:
			if(0 == strcmp(veriFils[0].type.class->name,"Interger") && 0 == strcmp(veriFils[1].type.class->name,"Interger")){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
			
		case MIN:
			if(0 == strcmp(veriFils[0].type.class->name,"Interger") && 0 == strcmp(veriFils[1].type.class->name,"Interger")){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
		
		case MULT:
			if(0 == strcmp(veriFils[0].type.class->name,"Interger") && 0 == strcmp(veriFils[1].type.class->name,"Interger")){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
		
		case DIVI:
			if(0 == strcmp(veriFils[0].type.class->name,"Interger") && 0 == strcmp(veriFils[1].type.class->name,"Interger")){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
			
		case AND:
			if(0 == strcmp(veriFils[0].type.class->name,"String") && 0 == strcmp(veriFils[1].type.class->name,"String")){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
			
		case CST:
			result.type.class = FindClass(env->listClass, "Integer");
			return result;
			break;
		
		case _ID:
			if(noeud->u.str[0] <= 91){
				result.type.class = FindClass(env->listClass, noeud->u.lvar->name);
				if(result.type.class == NIL(t_class)) result.type.object = FindObject(env->listObj, noeud->u.lvar->name);
			}else{
				result.type.class = noeud->u.lvar->coeur->_type;
			}
			/*TODO*/
			return result;
			break;
			
		case _STR:
			result.type.class = FindClass(env->listClass, "String");
			return result;
			break;
			
		case CAST: /* TODO */
			if(AEstSuperDeB(getChild(noeud, 0)->u.str, veriFils[1].type.class->name, env)){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
		
		case E_SELECT: /* L'espression est faite dans la verif de portée */
			result.type.class = veriFils[1].type.class;
			return result;
			break;
		
		case E_CALL_METHOD: /*  */
			m = veriFils[1].type.class->methods;
			while(m != NIL(t_method)){
				if(0 == strcmp(m->name, getChild(noeud, 1)->u.lvar->name)){
					result.type.class = m->returnType;
				}
				m = m->next;
			}
			return result;
			break;
			
		case LIST_ARG: 
			return verifTypageSuccesFils(noeud->nbChildren, noeud, env);
			
		case INST: /* TODO */
			result.type.class = veriFils[0].type.class;
			return result;
			break;
		
		default:
			printf("ERRRRRRRRRRRRRROOOORRRRRRR	%d\n",noeud->op);
			break;
			
	}
	
	result.succes = 0;
	return result;
}

Vtypage verifTypageSuccesFils(short nbre, TreeP noeud, list_ClassObjP env){
	Vtypage res;
	res.succes = 0;
	
	int i;
	for(i=0 ; i<nbre ; i++){
		if(!(verifcationTypageNoeud(getChild(noeud, i), env).succes)){return res;}
	}
	
	res.succes = 1;
	return res;
}

Vtypage verifcationTypageListVarDecl(VarDeclP liste, list_ClassObjP env){
	
	Vtypage result;
	
	while(liste != NIL(VarDecl)){
		
		Vtypage expr = verifcationTypageNoeud(liste->coeur->value, env);
		
		if(!(expr.succes) || 0 != strcmp(expr.type.class->name, liste->coeur->_type->name)){
			result.succes = 0;
			return result;
		}
			
		liste = liste->next;
	}
	result.succes = 1;
	return result;
	
}

int AEstSuperDeB(char* A, char* B,list_ClassObjP env){
	t_class* i = FindClass(env->listClass, B);
	
	while(i != NIL(t_class)){
		if(0 == strcmp(i->name,A)){
		 return 1;
		}
		i = i->superClass;
	}
	return 0;
}

int verificationTypageMethode(t_class* C, t_method* method, list_ClassObjP env){
	while(method != NIL(t_method)){
		if (!(verifcationTypageListVarDecl(method->parametres, env).succes)) return 0;
		if (!(verifcationTypageNoeud(method->bloc, env).succes)) return 0;
		
		/* TYPE DE RETOUR OK ? */
		Vtypage okko;
		okko.type.class = getReturnType(method->bloc, env);
		if(0 != strcmp(method->returnType->name, okko.type.class->name)) return 0;
		
		/*****ON VERIFIE SI LA METHODE EST BIEN REDEFINIE*****/
		if(method->isRedef){
			
			t_class* Ci = C->superClass;
			int valide = 0;
			
			while(Ci != NIL(t_class)){
				t_method* i = Ci->methods;
				
				while(i != NIL(t_method)){
					if(0 == strcmp(i->name, method->name)
						&& i->nbParametres == method->nbParametres
						&& i->returnType == method->returnType){
							
							VarDeclP j = i->parametres;
							VarDeclP h = method->parametres;
							
							while(j != NIL(VarDecl)){
								if(0 == strcmp(j->name, h->name) && 0 == strcmp(j->coeur->_type->name, h->coeur->_type->name)){
										valide = 1;
									}
								j = j->next;
								h = h->next;
							}
					}
					i = i->next;
				}
				Ci = Ci->superClass;
			}
			if(!valide) return 0;
		}
		
	}
	return 1;
}

int verificationTypageMethodeO(t_method* method, list_ClassObjP env){
	while(method != NIL(t_method)){
		if (!(verifcationTypageListVarDecl(method->parametres, env).succes)) return 0;
		if (!(verifcationTypageNoeud(method->bloc, env).succes)) return 0;
		
		/* TYPE DE RETOUR OK ? */
		Vtypage okko;
		okko.type.class = getReturnType(method->bloc, env);
		if(0 != strcmp(method->returnType->name, okko.type.class->name)) return 0;

		if(method->isRedef) return 0;
	}
	return 1;
}

int verificationTypageEnvironnement(list_ClassObjP env){
	t_class* i = env->listClass;
	t_object* j = env->listObj;
	
	while(i != NIL(t_class)){
		if (!(verifcationTypageListVarDecl(i->parametres, env).succes)) return 0;
		if (!(verifcationTypageListVarDecl(i->attributes, env).succes)) return 0;
		if (!(verificationTypageMethode(i, i->methods, env))) return 0;
		if (!(verificationTypageMethode(i, i->constructor, env))) return 0;
		
		i = i->next;
	}
	
	while(j != NIL(t_object)){
		if (!(verifcationTypageListVarDecl(j->attributes, env).succes)) return 0;
		if (!(verificationTypageMethodeO(j->methods, env))) return 0;
		
		j = j->next;
	}
	
	return 1;
}

int verificationTypage(list_ClassObjP env, TreeP core){
	return (verifcationTypageNoeud(core, env).succes && verificationTypageEnvironnement(env));
}

t_class* getReturnType(TreeP tree, list_ClassObjP env){
	if(tree->op == I_RETURN){
		return getChild(tree, 0)->u.lvar->coeur->_type;
	}else if(tree->nbChildren != 0){
		t_class* res = NIL(t_class);
		
		int i;
		for(i=0 ; i<tree->nbChildren ; i++){
			t_class* ite = getReturnType(getChild(tree, i), env);
			if(ite != NIL(t_class)){
				if(res == NIL(t_class)){
					res = ite;
				}else{
					if(0 == strcmp(res->name, ite->name)) return res;
					return NIL(t_class);
				}
			}else{
				return NIL(t_class);
			}
		}
		return res;
	}else{
		return FindClass(env->listClass, "Void");
	}
}

/*
bool verificationParametres(TreeP block){
		TreeP tree = block;
		bool toReturn = TRUE;
		while(tree != NIL(Tree)){
			if(tree->op == E_CALL_METHOD){

				t_class* c = getChild(tree,3)->u.lvar->coeur->_type;

				while(strcmp(getChild(tree,3)->u.str,c->methods->name) != 0){
					if(c->methods == NIL(t_method)){
						setError(NO_EXISTING_METHOD);
					}
					c->methods = c->methods->next;
				}
				
				t_method* decl = c->methods;
				VarDeclP entry = getChild(tree,4)->u.lvar;
				int givenNb = 0;
				while(entry != NULL){
					entry = entry->next;
					givenNb++;
				}
				toReturn = toReturn && ((decl->nbParametres == givenNb) ? TRUE : FALSE);

				if(toReturn == FALSE){
					setError(PARAM_ERROR_1);
				}
				else{

				

					entry = getChild(tree,4)->u.lvar;
					VarDeclP PDecl = decl->parametres;
					while(entry != NIL(VarDecl) || PDecl !=NIL(VarDecl)){

						toReturn = toReturn && ((strcmp(PDecl->coeur->_type->name,entry->coeur->_type->name) == 0) ? TRUE : FALSE);
						if(toReturn == FALSE){
							setError(PARAM_ERROR_2);
						}
						entry = entry->next;
						PDecl = PDecl->next; 
					}
				}
				tree = getChild(tree,1);	
			}
			else if(tree->op == INST){

				t_method* constructor = getChild(tree,1)->u.lvar->coeur->_type->constructor;
				if(strcmp(constructor->name,getChild(tree,1)->u.str) == 0){

					
					VarDeclP entry = getChild(tree,4)->u.lvar;
					int givenNb = 0;
					while(entry != NULL){
						entry = entry->next;
						givenNb++;
					}
					toReturn = toReturn && ((constructor->nbParametres == givenNb) ? TRUE : FALSE);

					if(toReturn == FALSE){
					setError(PARAM_ERROR_1);
					}
					else{

					

						entry = getChild(tree,4)->u.lvar;
						VarDeclP PDecl = constructor->parametres;
						while(entry != NIL(VarDecl) || PDecl !=NIL(VarDecl)){

							toReturn = toReturn && ((strcmp(PDecl->coeur->_type->name,entry->coeur->_type->name) == 0) ? TRUE : FALSE);
							if(toReturn == FALSE){
								setError(PARAM_ERROR_2);
							}
							entry = entry->next;
							PDecl = PDecl->next; 
					}
				}
					return toReturn;
				}
				else setError(NO_EXISTING_METHOD);
			}
		}
		return toReturn;
}*/

bool verificationBoucleHeritage(list_ClassObjP env, t_class* class){

	t_class* temp = class;

	while(temp != NIL(t_class)){

		if(AEstSuperDeB(class->name,temp->superClass->name,env) == 0){
			printf("%s et %s forment une boucle d'heritage : KO\n", class->name,temp->name);
			return FALSE;
		}

		temp = temp->superClass;
	}

	return TRUE;
}

bool verificationNomClasse(list_ClassObjP env, char* name){
	
	int DejaVu = 0;
	t_class* temp1 = env->listClass;
	t_object* temp2 = env->listObj;

	while(temp1 != NIL(t_class)){
		if(strcmp(temp1->name,name) == 0){
			if(DejaVu){
				printf("%s : Nom classe deja existant\n", name);
				return FALSE;
			}else{
				DejaVu = 1;
			}
		}
		temp1 = temp1->next;
	}
	
	while(temp2 != NIL(t_object)){
		if(strcmp(temp2->name,name) == 0){
			if(DejaVu){
				printf("%s : Nom classe deja existant\n", name);
				return FALSE;
			}else{
				DejaVu = 1;
			}
		}
		temp2 = temp2->next;
	}

	return TRUE;
}

bool verificationNomVarDecl(VarDeclP env, char* name){
	
	int DejaVu = 0;
	while(env != NIL(VarDecl)){
		if(strcmp(env->name,name) == 0){
			if(DejaVu){
				printf("%s : Nom attribut deja existant\n", name);
				return FALSE;
			}else{
				DejaVu = 1;
			}
		}
		env = env->next;
	}
	return TRUE;
}

bool verificationNomMethod(t_method* env, char* name){
	
	int DejaVu = 0;
	while(env != NIL(t_method)){
		if(strcmp(env->name,name) == 0){
			if(DejaVu){
				printf("%s : Nom methode deja existant\n", name);
				return FALSE;
			}else{
				DejaVu = 1;
			}
		}
		env = env->next;
	}
	return TRUE;
}

VarDeclP InitialisationSuperThisResultC(t_method* method, t_class* class, VarDeclP param){
	
	/**		CREATION DE this	*/
	VarDeclP this = NEW(1, VarDecl);
	this->coeur = NEW(1, t_variable);
	this->coeur->_type = class;
	this->name = "this";
	
	/**		CREATION DE result	*/
	VarDeclP result = NIL(VarDecl);
	if(0 != strcmp(method->returnType->name,"Void")){
		result = NEW(1, VarDecl);
		result->coeur = NEW(1, t_variable);
		result->coeur->_type = method->returnType;
		result->name = "result";
		
	}
	
	/**		CREATION DE super	*/
	VarDeclP super = NIL(VarDecl);
	if(class->superClass != NIL(t_class)){
		super = NEW(1, VarDecl);
		super->coeur = NEW(1, t_variable);
		super->coeur->_type = class->superClass;
		super->name = "result";
	}
	
	if(super != NIL(VarDecl) && result != NIL(VarDecl)){
		this->next = super;
		super->next = result;
		result->next = param;
	}else if(result != NIL(VarDecl)){
		this->next = result;
		result->next = param;
	}else if(super != NIL(VarDecl)){
		this->next = super;
		super->next = param;
	}
	
	return this;
}

VarDeclP InitialisationSuperThisResultO(t_method* method, t_object* object, VarDeclP param){
	
	/**		CREATION DE this	*/
	VarDeclP this = NEW(1, VarDecl);
	this->coeur = NEW(1, t_variable);
	this->coeur->_obj = object;
	this->name = "this";
	
	/**		CREATION DE result	*/
	VarDeclP result = NIL(VarDecl);
	if(0 != strcmp(method->returnType->name,"Void")){
		result = NEW(1, VarDecl);
		result->coeur = NEW(1, t_variable);
		result->coeur->_type = method->returnType;
		result->name = "result";
		
	}

	if(result != NIL(VarDecl)){
		this->next = result;
		result->next = param;
	}
	
	return this;
}

void afficheListVarDeclP(VarDeclP liste){
	printf("Variables\n");
	while(liste != NIL(VarDecl)){
		printf("%s..",liste->name);
		liste = liste->next;
	}
	printf("\n");
}
