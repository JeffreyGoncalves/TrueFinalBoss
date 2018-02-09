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
		bool isCorrect = verifPorteeClassObj(classObjList);
		 
		printf("[[[[[[[[[[[[[[[[[[[L'analyse de portée de l'environement est donc %d]]]]]]]]]]]]]]]]]]]\n",isCorrect);
		
		isCorrect = isCorrect && verifPorteeBloc(tree, NULL, classObjList);
		
		printf("[[[[[[[[[[[[[[[[[[[L'analyse de portée du corps est donc %d]]]]]]]]]]]]]]]]]]]\n",isCorrect);

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
	bool toReturn = TRUE;
	/*afficheListVarDeclP(listDecl);*/
	printf("\nInstruction # ");
		/* Bloc */
		if(inst->op == I_BLOC) toReturn = verifPorteeBloc(inst, listDecl, classObjList);
			
		/* Return, non autorise ici s'il n'existe pas de VarDecl avec le nom "result" dans la liste. */
		else if(inst->op == I_RETURN)
		{
			printf("return\n");
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
			printf("fin_return\n");
		}
		
		/* ITE */
		else if(inst->op == I_ITE)
		{
			printf("ite\n");
			TreeP Expr = getChild(inst, 0),
				  Inst1 = getChild(inst, 1),
				  Inst2 = getChild(inst, 2);
			/*toReturn = verifPorteeExpr(Expr, listDecl, classObjList)
					&& verifPorteeInst(Inst1, listDecl, classObjList) 
					&& verifPorteeInst(Inst2, listDecl, classObjList);*/
			toReturn = verifPorteeExpr(Expr, listDecl, classObjList);
			/*printf("condition [%d]\n", toReturn);*/
			toReturn = toReturn && verifPorteeInst(Inst1, listDecl, classObjList);
			/*printf("then [%d]\n", toReturn);*/
			toReturn = toReturn && verifPorteeInst(Inst2, listDecl, classObjList);
			/*printf("else [%d]\n", toReturn);*/
			
			printf("fin_ite\n");
		}
		
		/* Affectation */
		else if(inst->op == I_AFF)
		{
			printf("aff\n");
			TreeP Obj = getChild(inst, 0),
				  Expr = getChild(inst, 1);
			toReturn = verifPorteeExpr(Obj, listDecl, classObjList)
					&& verifPorteeExpr(Expr, listDecl, classObjList);
			/*printf("%d et %d\n",verifPorteeExpr(Obj, listDecl, classObjList),verifPorteeExpr(Expr, listDecl, classObjList));
			*/printf("fin_aff\n");
		}
		
		/* Expression */
		else if(inst->op == I_EXPRRELOP)
		{
			printf("relop\n");
			toReturn = verifPorteeExpr(getChild(inst, 0), listDecl, classObjList);
			printf("relop_fin\n");
		}  
		
		else
		{
			printf("rien\n");
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

	/** La super classe est déjà vérifiée avant. */

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
						printf("%d Attributs\n",toReturn);
						methodBuffer = methodBuffer->next;
					}
					methodBuffer = class->methods;
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
	
	if(strcmp(class->name, "String") && strcmp(class->name, "Integer") && strcmp(class->name, "Void")){
		if(!verifPorteeBloc(method->bloc, InitialisationSuperThisResultC(method, class, method->parametres), classObjList)) toReturn = FALSE;
	}
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
	printf("		%d paramètres\n",toReturn);
	
	/* le type de retour est déjà vérifié de par la génération des structures. */
	
	/*  Idem pour le nom. */
	
	if(strcmp(class->name, "String") && strcmp(class->name, "Integer") && strcmp(class->name, "Void")){
		if(!verifPorteeBloc(method->bloc, InitialisationSuperThisResultC(method, class, method->parametres), classObjList)) toReturn = FALSE;
	}
	printf("		%d bloc ok\n",toReturn);
	
	return toReturn;
}

bool verifPorteeBloc(TreeP tree, VarDeclP listDecl, list_ClassObjP classObjList)
{	
	afficheListVarDeclP(listDecl);
	if(tree->op == I_BLOC){
		
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
		
		afficheListVarDeclP(listDecl);
		
		/*** Verification des instructions (portee) ***/
		if(listInst != NULL)
		{
			while(listInst->nbChildren == 2)
			{
				toReturn = toReturn && verifPorteeInst(getChild(listInst, 0), listDecl, classObjList);
				listInst = getChild(listInst, 1);
			}
			toReturn = toReturn && verifPorteeInst(getChild(listInst, 0), listDecl, classObjList);
		}
		/**********************************************/

		return toReturn;
	}
	else return verifPorteeExpr(tree, listDecl, classObjList);
}

/* A TRAVAILLER */
bool verifPorteeExpr(TreeP Expr, VarDeclP listDecl, list_ClassObjP classObjList)
{
	printf("Expression # ");
	
	bool toReturn = TRUE;
	int i; /* Sert de variable de parcours */
	
	/* Si c'est un noeud NIL */
	if(Expr == NIL(Tree)){
		printf("noeud NIL\n");
		return toReturn;
	}
	
	/* Expression feuille */
	if(Expr->nbChildren == 0)
	{
		/*ID (les variables crees plus recemment sont prioritaires) */
		if(Expr->op == _ID)
		{
			printf("id # %s\n",Expr->u.lvar->name);
			/* Variables Crees dans le bloc */
			VarDeclP varSel = listDecl;
			while(varSel != NULL)
			{
				if(!strcmp(varSel->name, Expr->u.lvar->name))
				{
					free(Expr->u.lvar);
					Expr->u.lvar = varSel;
					printf("fin_id\n");
					return TRUE;
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
					printf("fin_id\n");
					return TRUE;
				}
				objSel = objSel->next;
			}
			
			/* Aucun objet n'a ete trouve */
			printf("fin_id\n");
			setError(VAR_NOT_FOUND);
			toReturn = FALSE;
		}
		else {printf("solo_pas_id\n");
			return TRUE;}
	}
	
	else
	{
		t_class* classBuffer = NIL(t_class);
		t_object* objectBuffer = NIL(t_object);
		bool estTrouver = FALSE;
		
		switch(Expr->op){
			case INST:
				printf("instantation\n");
				toReturn = toReturn && verifPorteeExpr(getChild(Expr, 1), listDecl, classObjList);
				classBuffer = FindClass(classObjList->listClass, getChild(Expr, 0)->u.lvar->name);
				if(classBuffer == NIL(t_class)){
					setError(CLASS_NOT_FOUND);
					toReturn = FALSE;
				}
				else{
					getChild(Expr, 0)->u.lvar->coeur->_type = classBuffer;
				}
				printf("instantation_fin\n");
				break;
				
			case E_SELECT:
				printf("selection\n");
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
					
					switch(getChild(Expr, 0)->op){
						case _ID:
							classBuffer = getChild(Expr, 0)->u.lvar->coeur->_type;
							objectBuffer = getChild(Expr, 0)->u.lvar->coeur->_obj;
							
							if(classBuffer != NIL(t_class)) printf("%s(%s)\n",getChild(Expr, 0)->u.lvar->name,classBuffer->name);
							else printf("%s(%s)\n",getChild(Expr, 0)->u.lvar->name,objectBuffer->name);
							break;
							
						case E_SELECT:
							classBuffer = getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type;
							printf("%s(%s)\n",getChild(getChild(Expr, 0), 1)->u.lvar->name,classBuffer->name);
							break;
							
						case E_CALL_METHOD:
							if(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type != NIL(t_class)){
								classBuffer = getReturnC(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type, getChild(getChild(Expr, 0), 1)->u.lvar->name, classObjList);
							}else{
								classBuffer = getReturnO(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_obj, getChild(getChild(Expr, 0), 1)->u.lvar->name, classObjList);
							}
							printf("%s(%s)\n",getChild(getChild(Expr, 0), 1)->u.lvar->name,classBuffer->name);
							break;
						
						case AND:
						case _STR:
							classBuffer = FindClass(classObjList->listClass, "String");
							if(getChild(Expr, 0)->op == _STR) printf("%s(String)\n",getChild(Expr, 0)->u.str);
							else printf("Expr(String)\n");
							break;
						
						case NE:
						case EQ:
						case LT:
						case LE:
						case GT:
						case GE:
						case DIVI:
						case MULT:
						case SUM:
						case MIN:	
						case CST:
							classBuffer = FindClass(classObjList->listClass, "Integer");
							if(getChild(Expr, 0)->op == CST) printf("%d(Integer)\n",getChild(Expr, 0)->u.val);
							else printf("Expr(Integer)\n");
							break;
							
						case CAST:						
						case INST:
							classBuffer = getChild(getChild(Expr, 0), 0)->u.lvar->coeur->_type;
							if(getChild(Expr, 0)->op == CAST)printf("CAST(%s)\n",classBuffer->name);
							else printf("INST(%s)\n",classBuffer->name);
							break;
							
						default:
							setError(OTHER_CONTEXTUAL_ERROR);
							toReturn = FALSE;
							break;
					}
					
					if(classBuffer == NIL(t_class) && objectBuffer == NIL(t_object)){
						setError(CLASS_NOT_FOUND);
						toReturn = FALSE;
					}
				}
				
				/* On cherche si l'attribut exite bel et bien dans la classe/objet trouvé. */
				VarDeclP varBuffer, pointVar;
				
				if(classBuffer != NIL(t_class)){
					while(classBuffer != NIL(t_class)){
						varBuffer = classBuffer->attributes;
						
						while(varBuffer != NIL(VarDecl)){
							if (!strcmp(varBuffer->name, getChild(Expr, 1)->u.lvar->name)){
								if(!estTrouver) pointVar = varBuffer; /*Ce if permet de prendre la 1ère variable visible*/
								estTrouver = TRUE;
							}
							varBuffer = varBuffer->next;
						}
						
						classBuffer = classBuffer->superClass;
					}
				}else if(objectBuffer != NIL(t_object)){
					varBuffer = objectBuffer->attributes;

					while(varBuffer != NIL(VarDecl)){
						if (!strcmp(varBuffer->name, getChild(Expr, 1)->u.lvar->name)){
							estTrouver = TRUE;
							pointVar = varBuffer;
						}
						varBuffer = varBuffer->next;
					}
				}
				
				if(estTrouver == TRUE){
					getChild(Expr, 1)->u.lvar = pointVar;
					getChild(Expr, 1)->u.lvar = pointVar;
				}else{
					setError(VAR_NOT_FOUND);
					toReturn = FALSE;
				}
				printf("selection_fin\n");		
				break;
				
			case E_CALL_METHOD:
				printf("appel_methode\n");
				/* On cherche le ID_C, s'il existe ou non.*/
				if(getChild(Expr, 0)->op == _ID && getChild(Expr, 0)->u.lvar->name[0] <= 90){
					objectBuffer = FindObject(classObjList->listObj, getChild(Expr, 0)->u.lvar->name);
					printf("OBJECT\n");
					if(objectBuffer == NIL(t_object)){
						setError(CLASS_NOT_FOUND);
						toReturn = FALSE;
					}
					else{
						getChild(Expr, 0)->u.lvar->coeur->_obj = objectBuffer;
					}
					
				}else{/**		cas avec Object.ID() ou (ExprRelop).ID()		*/
					
					toReturn = toReturn && verifPorteeExpr(getChild(Expr, 0), listDecl, classObjList);
					
					switch(getChild(Expr, 0)->op){
						case _ID:
							classBuffer = getChild(Expr, 0)->u.lvar->coeur->_type;
							objectBuffer = getChild(Expr, 0)->u.lvar->coeur->_obj;
							
							if(classBuffer != NIL(t_class)) printf("%s(%s)\n",getChild(Expr, 0)->u.lvar->name,classBuffer->name);
							else printf("%s(%s)\n",getChild(Expr, 0)->u.lvar->name,objectBuffer->name);
							break;
							
						case E_SELECT:
							classBuffer = getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type;
							printf("%s(%s)\n",getChild(getChild(Expr, 0), 1)->u.lvar->name,classBuffer->name);
							break;
							
						case E_CALL_METHOD:
							if(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type != NIL(t_class)){
								classBuffer = getReturnC(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type, getChild(getChild(Expr, 0), 1)->u.lvar->name, classObjList);
							}else{
								classBuffer = getReturnO(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_obj, getChild(getChild(Expr, 0), 1)->u.lvar->name, classObjList);
							}
							printf("%s(%s)\n",getChild(getChild(Expr, 0), 1)->u.lvar->name,classBuffer->name);
							break;
						
						case AND:
						case _STR:
							classBuffer = FindClass(classObjList->listClass, "String");
							if(getChild(Expr, 0)->op == _STR) printf("%s(String)\n",getChild(Expr, 0)->u.str);
							else printf("Expr(String)\n");
							break;
						
						case NE:
						case EQ:
						case LT:
						case LE:
						case GT:
						case GE:
						case DIVI:
						case MULT:
						case SUM:
						case MIN:	
						case CST:
							classBuffer = FindClass(classObjList->listClass, "Integer");
							if(getChild(Expr, 0)->op == CST) printf("%d(Integer)\n",getChild(Expr, 0)->u.val);
							else printf("Expr(Integer)\n");
							break;
							
						case CAST:						
						case INST:
							classBuffer = getChild(getChild(Expr, 0), 0)->u.lvar->coeur->_type;
							if(getChild(Expr, 0)->op == CAST)printf("CAST(%s)\n",classBuffer->name);
							else printf("INST(%s)\n",classBuffer->name);
							break;
							
						default:
							setError(OTHER_CONTEXTUAL_ERROR);
							toReturn = FALSE;
							break;
					}

					if(classBuffer == NIL(t_class) && objectBuffer == NIL(t_object)){
						setError(CLASS_NOT_FOUND);
						toReturn = FALSE;
					}
				}
				
				/* On cherche si la methode exite bel et bien dans la classe/objet trouvé. */
				t_method* methodBuffer;
				
				if(classBuffer != NIL(t_class)){
					t_class* tempo = classBuffer;
					while(tempo != NIL(t_class)){
						methodBuffer = tempo->methods;
						
						while(methodBuffer != NIL(t_method)){
							if (!strcmp(methodBuffer->name, getChild(Expr, 1)->u.lvar->name)){
								if(!estTrouver) classBuffer = tempo; /* Ce if permet de prendre la bonne méthode en cas d'overrides */
								estTrouver = TRUE;
							}
							
							methodBuffer = methodBuffer->next;
						}
						
						tempo = tempo->superClass;
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

				printf("appel_methode_fin\n");			
				break;
				
			case CAST:
				/* On vérifie l'expression qui se fait cast.*/
				toReturn = toReturn && verifPorteeExpr(getChild(Expr, 1), listDecl, classObjList);
				
				/* On vérifie que le cast est bien une classe qui existe. */
				classBuffer = FindClass(classObjList->listClass, getChild(Expr, 0)->u.lvar->name);
				
				/* Aucune classe n'a ete trouvee */
				if(classBuffer == NIL(t_class)){
					setError(CLASS_NOT_FOUND);
					toReturn = FALSE;
				}else{
					getChild(Expr, 0)->u.lvar->coeur->_type = classBuffer;
				}
				break;

			default:
				printf("+ ou - ou * ou etc...\n");
				/*bool toReturn = TRUE;*/
				for(i=0;i<Expr->nbChildren;i++)
					toReturn = toReturn && verifPorteeExpr(getChild(Expr, i), listDecl, classObjList);
				printf("+ ou - ou * ou etc... _fin\n");
	
		}
	}
	
	return toReturn;
}
/******************************************************************************************/

Vtypage verifcationTypageNoeud(TreeP noeud, list_ClassObjP env){
	printf("Typage # ");
	
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
			printf("return\n");
			return result;
		
		case I_EXPRRELOP:
			printf("relop\n");
			return result;
			/*return verifTypageSuccesFils(noeud->nbChildren, noeud, env);*/
			
		case I_ITE:
			if(0 == strcmp(veriFils[0].type.class->name,"Integer")){
				return result;
			}
			break;
			
		case I_AFF:
			printf("aff\n");
			if(0 == strcmp(veriFils[0].type.class->name, veriFils[1].type.class->name)){
				return result;
			}
			break;
			
		case SUM:
			printf("+\n");
			if(0 == strcmp(veriFils[0].type.class->name,"Integer") && 0 == strcmp(veriFils[1].type.class->name,"Integer")){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
			
		case MIN:
			printf("-\n");
			if(0 == strcmp(veriFils[0].type.class->name,"Integer") && 0 == strcmp(veriFils[1].type.class->name,"Integer")){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
		
		case MULT:
			printf("*\n");
			if(0 == strcmp(veriFils[0].type.class->name,"Interger") && 0 == strcmp(veriFils[1].type.class->name,"Integer")){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
		
		case DIVI:
			printf("/\n");
			if(0 == strcmp(veriFils[0].type.class->name,"Interger") && 0 == strcmp(veriFils[1].type.class->name,"Integer")){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
			
		case AND:
			printf("&\n");
			if(0 == strcmp(veriFils[0].type.class->name,"String") && 0 == strcmp(veriFils[1].type.class->name,"String")){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
			
		case CST:
			printf("constante\n");
			result.type.class = FindClass(env->listClass, "Integer");
			return result;
			break;
		
		case _ID:
			printf("id\n");
			/*printf("%s   %c \n",noeud->u.lvar->name, noeud->u.lvar->name[0]);*/
			if(noeud->u.lvar->name[0] <= 91){
				result.type.class = FindClass(env->listClass, noeud->u.lvar->name);
				if(result.type.class == NIL(t_class)) result.type.object = FindObject(env->listObj, noeud->u.lvar->name);
			}else{
				result.type.class = noeud->u.lvar->coeur->_type;
			}
			/*TODO*/
			printf("id_t_fin\n");
			return result;
			break;
			
		case _STR:
			printf("string\n");
			result.type.class = FindClass(env->listClass, "String");
			return result;
			break;
			
		case CAST: /* TODO */
			printf("cast\n");
			if(AEstSuperDeB(getChild(noeud, 0)->u.str, veriFils[1].type.class->name, env)){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
		
		case E_SELECT: /* L'espression est faite dans la verif de portée */
			printf("selection\n");
			result.type.class = veriFils[1].type.class;
			return result;
			break;
		
		case E_CALL_METHOD: /*  */
			printf("call_method\n");
			m = veriFils[1].type.class->methods;
			printf("JE %s         \n",veriFils[1].type.class->name);
			while(m != NIL(t_method)){
				printf("JE %s         \n",m->name);
				if(0 == strcmp(m->name, getChild(noeud, 1)->u.lvar->name)){
					printf("JE PASSE ICI\n");
					result.type.class = m->returnType;
				}
				m = m->next;
			}
			printf("call_method_fin\n");
			return result;
			break;
			
		case LIST_ARG: 
			return verifTypageSuccesFils(noeud->nbChildren, noeud, env);
			
		case INST: /* TODO */
			printf("instantation\n");
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

bool verificationBoucleHeritage(t_class* class){

	char* nameDepart = class->name;
	
	class = class->superClass;
		
	while(class != NIL(t_class)){
		if(!strcmp(class->name,nameDepart)){
			printf("%s forme une boucle d'heritage : KO\n", class->name);
			return FALSE;
		}

		class = class->superClass;
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
		super->name = "super";
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
	}else{
		this->next = param;
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
	}else{
		this->next = param;
	}
	
	return this;
}

void afficheListVarDeclP(VarDeclP liste){
	printf("Variables\n");
	while(liste != NIL(VarDecl)){
		if(liste->coeur->_type != NIL(t_class)) printf("%s(%s)..",liste->name,liste->coeur->_type->name);
		else printf("%s(%s)..",liste->name,liste->coeur->_obj->name);

		liste = liste->next;
	}
	printf("\n");
}

t_class* getReturnC(t_class* class, char* nom_methode, list_ClassObjP env){
	t_method* i = class->methods;
	while(i != NIL(t_method)){
		if(!strcmp(i->name,nom_methode)) return FindClass(env->listClass, i->returnType->name);
		i = i->next;
	}
	return NIL(t_class); /* normalement n'arrive jamais.*/
}

t_class* getReturnO(t_object* object, char* nom_methode, list_ClassObjP env){
	t_method* i = object->methods;
	while(i != NIL(t_method)){
		if(!strcmp(i->name,nom_methode)) return FindClass(env->listClass, i->returnType->name);
		i = i->next;
	}
	return NIL(t_class); /* normalement n'arrive jamais.*/
}

bool verificationSuperClass(list_ClassObjP classObjList){
	bool toReturn = TRUE;
	t_class* classBuffer = classObjList->listClass;
	
	while(classBuffer != NIL(t_class)){
		printf("|||||||%s:\n",classBuffer->name);
		/*	Verification de la super-classe */
		if(classBuffer->superClass != NIL(t_class)){
			t_class* iteratorC = FindClass(classObjList->listClass, classBuffer->superClass->name);
			if(iteratorC == NIL(t_class)) 
			{
				setError(CLASS_NOT_FOUND);
				toReturn = FALSE;
			}
			free(classBuffer->superClass); /** C'était une classe temporaire.*/
			classBuffer->superClass = iteratorC;
		}
		printf("%d Super-classe\n",toReturn);
		printf("______________________________\n");
		classBuffer = classBuffer->next;
	}
	
	classBuffer = classObjList->listClass;
	while(classBuffer != NIL(t_class)){
		toReturn = toReturn && verificationBoucleHeritage(classBuffer);
		classBuffer = classBuffer->next;
	}
	
	printf("[[[[[[[[[[[[[[[[[[[L'analyse de Super-Classe est donc %d]]]]]]]]]]]]]]]]]]]\n",toReturn);
	return toReturn;	
}
