#include "verif.h"
#include "tp.h"

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

void setInitTrue(TreeP exprTree)
{
	printf("Set de l'init : OK\n");
	/* Cas Id */
	if(exprTree->op == _ID)
	{
		exprTree->u.lvar->isInit = TRUE;
	}
	
	/* Cas E_SELECT */
	else if(exprTree->op == E_SELECT) 
	{
		VarDeclP var = getChild(exprTree, 1)->u.lvar;
		var->isInit = TRUE;
	}
	
	/* Note : il n'est pas possible d'avoir un ID_C car impossible d'initialiser un Objet en declaration */
}

bool verifInit(TreeP exprTree)
{
	bool toReturn = TRUE;
	printf("Verification de l'init : ");
	
	/* Cas Id */
	if(exprTree->op == _ID)
	{
		if(!exprTree->u.lvar->isInit && strcmp(exprTree->u.lvar->name, "this") && strcmp(exprTree->u.lvar->name, "super"))
			toReturn = FALSE;
	}
	
	/* Cas Selection */
	else if(exprTree->op == E_SELECT)
	{
		VarDeclP var = getChild(exprTree, 1)->u.lvar;
		if(!var->isInit)
			toReturn = FALSE;
	}
	
	printf("%d\n", toReturn);
	return toReturn;
}

/* Verification contextuelle : Portee
 * 		Instantiation
 * */
bool verifPorteeInst(TreeP inst, VarDeclP listDecl, list_ClassObjP classObjList)
{
	bool toReturn = TRUE;

		/* Bloc */
		if(inst->op == I_BLOC) toReturn = verifPorteeBloc(inst, listDecl, classObjList);
			
		/* Return, non autorise ici s'il n'existe pas de VarDecl avec le nom "result" dans la liste. */
		else if(inst->op == I_RETURN)
		{
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
		
		/* If Then Else */
		else if(inst->op == I_ITE)
		{
			
			TreeP Expr = getChild(inst, 0),
				  Inst1 = getChild(inst, 1),
				  Inst2 = getChild(inst, 2);

			toReturn = verifPorteeExpr(Expr, listDecl, classObjList);
			toReturn = toReturn && verifPorteeInst(Inst1, listDecl, classObjList);
			toReturn = toReturn && verifPorteeInst(Inst2, listDecl, classObjList);
			
		}
		
		/* Affectation */
		else if(inst->op == I_AFF)
		{
			
			TreeP Obj = getChild(inst, 0),
				  Expr = getChild(inst, 1);

			toReturn = verifPorteeExpr(Obj, listDecl, classObjList)
					&& verifPorteeExpr(Expr, listDecl, classObjList);
					
		}
		
		/* Expression */
		else if(inst->op == I_EXPRRELOP)
		{
			
			toReturn = verifPorteeExpr(getChild(inst, 0), listDecl, classObjList);

		}  
		
		else
		{
			setError(CONTEXT_ERROR);
			toReturn = FALSE;
			/* Ce cas ne doit arriver */
		}

	return toReturn;
}

/* Verification contextuelle : Portee
 * 		Parcours classes & objets
 * */
bool verifPorteeClassObj(list_ClassObjP classObjList)
{
	/**		On verifie les classes puis les objets.	*/
	
	return verifPorteeListClass(classObjList)
		&& verifPorteeListObject(classObjList);
}

/* Verification contextuelle : Portee
 * 		Parcours objets
 * */
bool verifPorteeListObject(list_ClassObjP classObjList)
{
	/**		On parcours simplement les objets	*/
	
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

/* Verification contextuelle : Portee
 * 		Objet isole
 * */
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
		
		/**		On regarde si l'expression est correcte	*/
		printf("	%s\n",VarDeclBuffer->name);
		toReturn = toReturn && verifPorteeExpr(VarDeclBuffer->coeur->value, InitialisationSuperThisResultO(NIL(t_method), object, NIL(VarDecl)), classObjList);
		
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

/* Verification contextuelle : Portee
 * 		Parcours classes
 * */
bool verifPorteeListClass(list_ClassObjP classObjList)
{
	printf("********		Portée des classes		********\n");
	/* Tout le bloc à partir d'ici */
	bool toReturn = TRUE;
	int nbreDeClasse = 0;
	t_class* iterator = classObjList->listClass;
	
	while(iterator != NIL(t_class)){
		nbreDeClasse++;
		iterator = iterator->next;
	}
	
	nbreDeClasse--;
	/* jusqu'à ici permet simplement de lire la liste de classe en sens inverse. Sinon nous rencontrons des problèmes
	 * En effet, il y avait le risque que nous checkons une classe dépendante d'une autre. Or cette dernière n'a pas encore été initialisée
	 * car la liste est inversée de base. */
	
	/* On parcourt....*/
	while(nbreDeClasse >= 0)
	{
		iterator = classObjList->listClass;
		int i;
		for(i=0 ; i<nbreDeClasse ; i++) iterator = iterator->next;
				
		if(!verifPorteeClass(iterator, classObjList) && toReturn) toReturn = FALSE;
		nbreDeClasse--;
	}
	
	return toReturn;
}

/* Verification contextuelle : Portee
 * 		Classe
 * */
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
		
		/**		On regarde si l'expression est correcte	*/
		printf("	%s\n",VarDeclBuffer->name);
		toReturn = toReturn && verifPorteeExpr(VarDeclBuffer->coeur->value, InitialisationSuperThisResultC(NIL(t_method), class, NIL(VarDecl)), classObjList);

		VarDeclBuffer = VarDeclBuffer->next;
	}
	printf("%d Attributs\n",toReturn);
	
	/* Verification methodes */
	t_method* methodBuffer = class->methods;
					while(methodBuffer != NIL(t_method)){
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

/* Verification contextuelle : Portee
 * 		Methodes des classes
 * */
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
	
	method->returnType = ClassBuffer;
	printf("		%d type du return\n",toReturn);
	
	if(strcmp(class->name, "String") && strcmp(class->name, "Integer") && strcmp(class->name, "Void")){
		if(!verifPorteeBloc(method->bloc, InitialisationSuperThisResultC(method, class, method->parametres), classObjList)) toReturn = FALSE;
	}
	printf("		%d bloc ok\n",toReturn);
	
	return toReturn;
}

/* Verification contextuelle : Portee
 * 		Methodes des objets
 * */
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

/* Verification contextuelle : Portee
 * 		Constructeurs des classes
 * */
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

		VarDeclBuffer->coeur->_type = ClassBuffer;
		
		VarDeclBuffer = VarDeclBuffer->next;
	}
	printf("		%d paramètres\n",toReturn);
	
	/* le type de retour est déjà vérifié de par la génération des structures. */
	
	/*  Idem pour le nom. */
	
	/**		Les classes predefinies sont creees par nos soins, elles sont forcement justes.	*/
	if(strcmp(class->name, "String") && strcmp(class->name, "Integer") && strcmp(class->name, "Void")){
		if(!verifPorteeBloc(method->bloc, InitialisationSuperThisResultC(method, class, method->parametres), classObjList)) toReturn = FALSE;
	}
	printf("		%d bloc ok\n",toReturn);
	
	return toReturn;
}

/* Verification contextuelle : Portee
 * 		Bloc
 * */
bool verifPorteeBloc(TreeP tree, VarDeclP listDecl, list_ClassObjP classObjList)
{	
	/* Enlever les commentaires pour faire apparaitre la liste des variables visibles à l'entree des blocs.*/
	/*afficheListVarDeclP(listDecl);*/
	
	if(tree->op == I_BLOC){
		
		bool toReturn = TRUE;
		VarDeclP listVarDecl = getChild(tree, 0)->u.lvar, tempo = NIL(VarDecl);
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
			
			
			/** ICI on regarde l'expression d'initialisation de variable. **/
			if(tempo != NIL(VarDecl)){
				tempo->next = listDecl;

				toReturn = toReturn && verifPorteeExpr(iterator->coeur->value, listVarDecl, classObjList);
				tempo->next = iterator;
			}else{
				toReturn = toReturn && verifPorteeExpr(iterator->coeur->value, listDecl, classObjList);
			}
			tempo = iterator;
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

/* Verification contextuelle : Portee
 * 		Expression
 * */
bool verifPorteeExpr(TreeP Expr, VarDeclP listDecl, list_ClassObjP classObjList)
{
	
	bool toReturn = TRUE;
	int i; /* Sert de variable de parcours */
	
	/* Si c'est un noeud NIL */
	if(Expr == NIL(Tree)){
		return toReturn;
	}
	
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
					return TRUE;
				}
				objSel = objSel->next;
			}
			
			/* Aucun objet n'a ete trouve */

			setError(VAR_NOT_FOUND);
			toReturn = FALSE;
		}
		else {return TRUE;}
	}
	
	else
	{
		t_class* classBuffer = NIL(t_class);
		t_object* objectBuffer = NIL(t_object);
		bool estTrouver = FALSE;
		
		switch(Expr->op){
			case INST:
				/* On vérifie les paramètres maintenant. */
				toReturn = toReturn && verifPorteeExpr(getChild(Expr, 1), listDecl, classObjList);
				
				classBuffer = FindClass(classObjList->listClass, getChild(Expr, 0)->u.lvar->name);
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
				}else{/*		cas avec Object.ID ou (ExprRelop).ID		*/
					
					toReturn = toReturn && verifPorteeExpr(getChild(Expr, 0), listDecl, classObjList);
					
					switch(getChild(Expr, 0)->op){
						case _ID:
							classBuffer = getChild(Expr, 0)->u.lvar->coeur->_type;
							objectBuffer = getChild(Expr, 0)->u.lvar->coeur->_obj;
							break;
							
						case E_SELECT:
							classBuffer = getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type;
							break;
							
						case E_CALL_METHOD:
							if(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type != NIL(t_class)){
								classBuffer = getReturnC(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type, getChild(getChild(Expr, 0), 1)->u.lvar->name, classObjList);
							}else{
								classBuffer = getReturnO(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_obj, getChild(getChild(Expr, 0), 1)->u.lvar->name, classObjList);
							}
							break;
						
						case AND:
						case _STR:
							classBuffer = FindClass(classObjList->listClass, "String");
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
							break;
							
						case CAST:						
						case INST:
							classBuffer = getChild(getChild(Expr, 0), 0)->u.lvar->coeur->_type;
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
				break;
				
			case E_CALL_METHOD:

				/* On cherche le ID_C, s'il existe ou non.*/
				if(getChild(Expr, 0)->op == _ID && getChild(Expr, 0)->u.lvar->name[0] <= 90){
					objectBuffer = FindObject(classObjList->listObj, getChild(Expr, 0)->u.lvar->name);
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
							
							break;
							
						case E_SELECT:
							classBuffer = getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type;
							break;
							
						case E_CALL_METHOD:
							if(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type != NIL(t_class)){
								classBuffer = getReturnC(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_type, getChild(getChild(Expr, 0), 1)->u.lvar->name, classObjList);
							}else{
								classBuffer = getReturnO(getChild(getChild(Expr, 0), 1)->u.lvar->coeur->_obj, getChild(getChild(Expr, 0), 1)->u.lvar->name, classObjList);
							}
							break;
						
						case AND:
						case _STR:
							classBuffer = FindClass(classObjList->listClass, "String");
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
							break;
							
						case CAST:						
						case INST:
							classBuffer = getChild(getChild(Expr, 0), 0)->u.lvar->coeur->_type;
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
				
				/* On vérifie les paramètres maintenant. */
				toReturn = toReturn && verifPorteeExpr(getChild(Expr, 2), listDecl, classObjList);
							
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
				/**		Repressente tous les autres cas inutile a traiter.	*/
				for(i=0;i<Expr->nbChildren;i++)
					toReturn = toReturn && verifPorteeExpr(getChild(Expr, i), listDecl, classObjList);

	
		}
	}
	
	return toReturn;
}
/******************************************************************************************/

/* Verification contextuelle : Typage
 * 		Noeud de l'arbre
 * */
Vtypage verifcationTypageNoeud(TreeP noeud, list_ClassObjP env){
	
	Vtypage result;
	Vtypage veriFils[5];
	int i;
	t_method* m = NIL(t_method);
	result.succes = 1;
	
	/*		Si le noeud est NIL, il est forcement juste car il n'y a pas de conflit de type.	*/
	if(noeud == NIL(Tree)){
		result.type.class = NIL(t_class);
		return result;
	}
	
	/*		On verifie que le typage est bon pour les fils. Grace a cela on peut connaitre le type d'une expression entiere.	*/
	for(i=0 ; i<noeud->nbChildren ; i++){
		veriFils[i] = verifcationTypageNoeud(getChild(noeud, i), env);
		if(!(veriFils[i].succes)){
			result.succes = 0;
			return result;
		}
	}
	
	/*		On traite les differents cas.	*/
	t_class *exprClass = veriFils[1].type.class;
	switch(noeud->op){
		
		case I_BLOC:
			return result;
		
		case DECL:
			return verifcationTypageListVarDecl(noeud->u.lvar, env);
			
		case LIST_PARAM:
			return verifcationTypageListVarDecl(noeud->u.lvar, env);
			
		case LIST_INST:
			return result;

		case I_RETURN:
			return result;
		
		case I_EXPRRELOP:
			return result;
			
		case I_ITE:
			if(0 == strcmp(veriFils[0].type.class->name,"Integer")){
				return result;
			}
			break;
			
		case I_AFF:
			while(exprClass != NIL(t_class))
			{
				if(0 == strcmp(veriFils[0].type.class->name, exprClass->name)){
					return result;
				}
				exprClass = exprClass->superClass;
			}
			break;
		
		case NE:
		case EQ:
		case LT:
		case LE:
		case GT:
		case GE:
		case SUM:
		case MIN:
		case MULT:
		case DIVI:
			
			if(noeud->nbChildren == 1){
				/**		CAS opérateur unaire - et +.	*/
				if(0 == strcmp(veriFils[0].type.class->name,"Integer")){
					result.type.class = veriFils[0].type.class;
					return result;
				}
			}else{
				/**		Autres cas		*/
				if(0 == strcmp(veriFils[0].type.class->name,"Integer") && 0 == strcmp(veriFils[1].type.class->name,"Integer")){
					result.type.class = veriFils[0].type.class;
					return result;
				}
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
			
			if(noeud->u.lvar->name[0] <= 91){
				result.type.class = FindClass(env->listClass, noeud->u.lvar->name);
				if(result.type.class == NIL(t_class)){
					result.type.object = FindObject(env->listObj, noeud->u.lvar->name);
				}
			}else{
				/** Ce if ci-dessous est obligatoire, car sinon Seg Fault sur "Object.f()" : il y a un pointeur vers t_object et pas vers t_class */
				if(noeud->u.lvar->coeur->_type != NIL(t_class)){
					result.type.class = noeud->u.lvar->coeur->_type;
				}else{
					result.type.object = noeud->u.lvar->coeur->_obj;
				}
			}
			return result;
			break;
		
		case _STR:
			result.type.class = FindClass(env->listClass, "String");
			return result;
			break;
			
		case CAST:			
			if(AEstSuperDeB(getChild(noeud, 0)->u.lvar->name, veriFils[1].type.class->name, env)){
				result.type.class = veriFils[0].type.class;
				return result;
			}
			break;
		
		case E_SELECT: /* L'espression est faite dans la verif de portée */
			result.type.class = veriFils[1].type.class;
			return result;
			break;
		
		case E_CALL_METHOD:			
			if(getChild(noeud, 0)->op == _ID){
				if(getChild(noeud, 0)->u.lvar->name[0] <= 91){	
					m = veriFils[1].type.object->methods;
				}
			}
			if(m == NIL(t_method)){
				m = veriFils[1].type.class->methods;
			}
			
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
			
		case INST:
			result.type.class = veriFils[0].type.class;
			return result;
			break;
		
		default:
			printOP(noeud->op);
			setError(CONTEXT_ERROR);
			break;
			
	}
	
	printOP(noeud->op);
	setError(CONTEXT_ERROR);
	result.succes = 0;
	return result;
}

/* Verification contextuelle : Typage
 * 		Parcours des fils d'un noeud.
 * */
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

/* Verification contextuelle : Typage
 * 		Liste de VarDeclP chainés
 * */
Vtypage verifcationTypageListVarDecl(VarDeclP liste, list_ClassObjP env){
	
	Vtypage result;
	
	while(liste != NIL(VarDecl)){
		
		/**		On verifie son expression	*/
		Vtypage expr = verifcationTypageNoeud(liste->coeur->value, env);
		
		/**		On verifie que cela correspond bien avec son type.	*/
		if(!(expr.succes)){
			result.succes = 0;
			return result;
		}else if(expr.type.class != NIL(t_class)){
			if(strcmp(expr.type.class->name, liste->coeur->_type->name)){
				result.succes = 0;
				return result;
			}
		}
		liste = liste->next;
	}
	result.succes = 1;
	return result;
	
}

int AEstSuperDeB(char* A, char* B,list_ClassObjP env){
	t_class* i = FindClass(env->listClass, B);
	
	while(i != NIL(t_class)){
		if(!strcmp(i->name,A)){
		 return 1;
		}
		i = i->superClass;
	}
	return 0;
}

/* Verification contextuelle : Typage
 * 		Methodes de classes
 * */
bool verificationTypageMethode(t_class* C, t_method* method, list_ClassObjP env){
	bool toReturn = TRUE;
	
	while(method != NIL(t_method)){
		printf("\n\n	Typage de [%s]\n", method->name);
		
		toReturn = (verifcationTypageListVarDecl(method->parametres, env).succes);
		printf("		Paramètres : %d\n", toReturn);
		
		toReturn = toReturn && (verifcationTypageNoeud(method->bloc, env).succes);
		printf("		Corps : %d\n", toReturn);
		
		/* TYPE DE RETOUR OK ? */
		t_class* classBuffer;
		if(method->bloc->op != I_BLOC){
			/** Cas 1 : la methode est de type 1 (sans bloc)*/
			
			classBuffer = verifcationTypageNoeud(method->bloc, env).type.class;
			if(classBuffer == NIL(t_class)) classBuffer = FindClass(env->listClass, "Void");
		
		}else{
			/** Cas 2 : la methode est de type 2 (avec bloc)*/
			classBuffer = getReturnType(method->bloc, env);
			if(!strcmp(classBuffer->name, "Void") && strcmp(method->returnType->name, "Void")){
				if(isDeclared(method->bloc, "result")) classBuffer = method->returnType;
			}
		}
		
		printf("		On attend [%s], on obtient [%s]\n", method->returnType->name, classBuffer->name);
		toReturn = toReturn && !strcmp(method->returnType->name, classBuffer->name);
		printf("		Type de retour : %d\n", toReturn);

		
		/*****ON VERIFIE SI LA METHODE EST BIEN REDEFINIE*****/
		int valide;
		if(method->isRedef){
			
			t_class* Ci = C->superClass;
			valide = 0;
			
			while(Ci != NIL(t_class)){
				t_method* i = Ci->methods;
				
				while(i != NIL(t_method)){
					if(0 == strcmp(i->name, method->name)
						&& i->nbParametres == method->nbParametres
						&& i->returnType == method->returnType){
							
							VarDeclP j = i->parametres;
							VarDeclP h = method->parametres;
							
							if(i->nbParametres == method->nbParametres){
								/**		Ne marche pas si nbParametres = 0	*/
								while(j != NIL(VarDecl)){
									if(0 == strcmp(j->name, h->name) && 0 == strcmp(j->coeur->_type->name, h->coeur->_type->name)){
											valide = 1;
										}
									j = j->next;
									h = h->next;
								}
								
								/**		Utilisee si nbParametres = 0	*/
								if(i->nbParametres == 0) valide = 1;
							}
					}
					i = i->next;
				}
				Ci = Ci->superClass;
			}
			toReturn = valide && toReturn;
			if(!valide) setError(OVERRIDE_ERROR);
		}
		
		printf("		Redefinition : %d\n", toReturn);
		method = method->next;
	}
	return toReturn;
}

/* Verification contextuelle : Typage
 * 		Methodes d'objets
 * */
bool verificationTypageMethodeO(t_method* method, list_ClassObjP env){
	bool toReturn;
	
	while(method != NIL(t_method)){
		printf("\n\n	Typage de [%s]\n", method->name);
		
		toReturn = (verifcationTypageListVarDecl(method->parametres, env).succes);
		printf("		Paramètres : %d\n", toReturn);
		
		toReturn = toReturn && (verifcationTypageNoeud(method->bloc, env).succes);
		printf("		Corps : %d\n", toReturn);
		
		/* TYPE DE RETOUR OK ? */
		t_class* classBuffer;
		if(method->bloc->op != I_BLOC){
			/** Cas 1 : la methode est de type 1 (sans bloc)*/
			
			classBuffer = verifcationTypageNoeud(method->bloc, env).type.class;
			if(classBuffer == NIL(t_class)) classBuffer = FindClass(env->listClass, "Void");
		
		}else{
			/** Cas 2 : la methode est de type 2 (avec bloc)*/
			classBuffer = getReturnType(method->bloc, env);
			if(!strcmp(classBuffer->name, "Void") && strcmp(method->returnType->name, "Void")){
				if(isDeclared(method->bloc, "result")) classBuffer = method->returnType;
				else setError(RETURN_ERROR);
			}
		}
		
		printf("		On attend [%s], on obtient [%s]\n", method->returnType->name, classBuffer->name);
		toReturn = toReturn && !strcmp(method->returnType->name, classBuffer->name);
		printf("		Type de retour : %d\n", toReturn);
		if(strcmp(method->returnType->name, classBuffer->name)) setError(RETURN_ERROR);


		if(method->isRedef){
			setError(OVERRIDE_ERROR);
			toReturn = FALSE;
		}
		
		method = method->next;
	}
	return toReturn;
}

/* Verification contextuelle : Typage
 * 		Constructeurs de classes
 * */
bool verificationTypageConstructeur(t_class* C, t_method* method, list_ClassObjP env){
	bool toReturn;
	
	if(method == NIL(t_method)){
		printf("\n\n	Typage de [%s] : Cette classe ne possede pas de constructeur.\n", C->name);
		toReturn = TRUE;
	}else{
		printf("\n\n	Typage de [%s]\n", method->name);
			
		toReturn = (verifcationTypageListVarDecl(method->parametres, env).succes);
		printf("		Paramètres : %d\n", toReturn);
			
		toReturn = toReturn && (verifcationTypageNoeud(method->bloc, env).succes);
		printf("		Corps : %d\n", toReturn);
	}
	
	return toReturn;
}

/* Verification contextuelle : Typage
 * 		Ensemble des classes & des objets
 * */
bool verificationTypageEnvironnement(list_ClassObjP env){
	
	bool toReturn = TRUE;
	t_class* i = env->listClass;
	t_object* j = env->listObj;
	
	while(i != NIL(t_class)){
		printf("|||||||%s:\n",i->name);
		
		if(strcmp(i->name,"Void") && strcmp(i->name,"String") && strcmp(i->name,"Integer")){
			toReturn = toReturn && (verifcationTypageListVarDecl(i->parametres, env).succes);
			printf("%d Parametres\n",toReturn);
			
			toReturn = toReturn && (verifcationTypageListVarDecl(i->attributes, env).succes);
			printf("%d Attributs\n",toReturn);
			
			toReturn = toReturn && (verificationTypageMethode(i, i->methods, env));
			printf("%d Methodes\n",toReturn);

			toReturn = toReturn && (verificationTypageConstructeur(i, i->constructor, env));
			printf("%d Constructor\n",toReturn);
		}else{
			printf("%d Parametres\n",toReturn);
			printf("%d Attributs\n",toReturn);
			printf("%d Methodes\n",toReturn);
			printf("%d Constructor\n",toReturn);
		}
		i = i->next;
		printf("________________________________________\n");
	}
	
	while(j != NIL(t_object)){
		printf("|||||||%s:\n",j->name);
		
		toReturn = toReturn && (verifcationTypageListVarDecl(j->attributes, env).succes);
		printf("%d Attributs\n",toReturn);
		
		toReturn = toReturn && (verificationTypageMethodeO(j->methods, env));
		printf("%d Methods\n",toReturn);
		
		j = j->next;
		printf("________________________________________\n");
	}
	
	return toReturn;
}

/* Verification contextuelle : Typage
 * 		Environnement puis programme coeur
 * */
bool verificationTypage(list_ClassObjP env, TreeP core){
	bool toReturn;
	
	printf("********			Typage environnement			********\n");
	toReturn = verificationTypageEnvironnement(env);
	printf("[[[[[[[[[[[[[[[[[[[L'analyse de typage de l'environnement est donc %d]]]]]]]]]]]]]]]]]]]\n",toReturn);

	
	printf("********			Portée corps du programme			********\n");
	toReturn = toReturn && verifcationTypageNoeud(core, env).succes;
	printf("[[[[[[[[[[[[[[[[[[[L'analyse de typage du corps est donc %d]]]]]]]]]]]]]]]]]]]\n",toReturn);

	return toReturn;
}

/* Verification contextuelle : Typage
 * 		Vérification du type de retour.
 * */
t_class* getReturnType(TreeP tree, list_ClassObjP env){
	
	if(tree->nbChildren != 0){
		t_class* res = NIL(t_class);
		
		int i;
		for(i=0 ; i<tree->nbChildren ; i++){		
			if(getChild(tree, i) != NIL(Tree)){
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
		}
		return res;
	}else{
		return FindClass(env->listClass, "Void");
	}
}


bool verificationParametres(TreeP block){
		TreeP tree = block;
		bool toReturn = TRUE;
		int i=0;
		
		for(i=0;i<tree->nbChildren;i++){
		
			if(tree->op == E_CALL_METHOD){
				
				t_class* c = getChild(tree,1)->u.lvar->coeur->_type;

				while(strcmp(getChild(tree,1)->u.lvar->name,c->methods->name) != 0){
					
					if(c->methods == NIL(t_method)){
						setError(NO_EXISTING_METHOD);
					}
					c->methods = c->methods->next;
				}
				
				t_method* decl = c->methods;
				TreeP entry = NULL;
				printf("Methode : %s\n", decl->name);
				/* On verifie d'abord le nombre de parametres entres
				 * lors de l'appel methode. Dans le cas 0, il suffit
				 * de comparer avec le nombre requis. Sinon il faut
				 * aussi verifier le type des arguments */
				if(getChild(tree,2) == NIL(Tree)){
					/* Cas ou l'appel methode utilise 0 arguments */
					if(decl->nbParametres != 0){
						setError(PARAM_ERROR_1);
						toReturn = FALSE;
					}
				}
				else{
					/* Cas ou l'appel utilise au moins 1 argument */
					if(decl->nbParametres == 0) {
						setError(PARAM_ERROR_1);
						toReturn = FALSE;
					}
					else {
						int givenNb = 1;
						entry = getChild(tree,2);
						while(getChild(entry, 1) != NIL(Tree)){
							entry = getChild(entry, 1);
							givenNb++;
						}
						/* Verification de nombre */
						if(givenNb != decl->nbParametres) 
						{
							toReturn = FALSE;
							setError(PARAM_ERROR_1);
						}
						/* Succès sur le nombre d'arguments, verification de type */
						else
						{
							entry = getChild(tree,2);
							VarDeclP param = decl->parametres;
							do
							{
								TreeP argTree = getChild(entry, 0);
								/* On recupere le type de l'expression de l'argument actuel */
								switch(argTree->op)
								{
									case CST : /* Integer */
										if(strcmp(param->coeur->_type->name, "Integer"))
										{
											setError(PARAM_ERROR_2);
											toReturn = FALSE;
										}
										break;
									case _STR : /* String */
										if(strcmp(param->coeur->_type->name, "String"))
										{
											setError(PARAM_ERROR_2);
											toReturn = FALSE;
										}
										break;
									case _ID :
										if(argTree->u.lvar->coeur->_type == NIL(t_class))
										{
											setError(PARAM_ERROR_3);
											toReturn = FALSE;
										}
										else
										{
											t_class *argClass = argTree->u.lvar->coeur->_type;
											while(strcmp(argClass->name, param->coeur->_type->name))
											{
												if(argClass->superClass == NIL(t_class))
												{
													setError(PARAM_ERROR_2);
													toReturn = FALSE;
													break;
												}
												else argClass = argClass->superClass;
											}
										}
										break;
									case E_SELECT :
										argTree = getChild(argTree, 1);
										if(argTree->u.lvar->coeur->_type == NIL(t_class))
										{
											setError(PARAM_ERROR_3);
											toReturn = FALSE;
										}
										else
										{
											t_class *argClass = argTree->u.lvar->coeur->_type;
											while(strcmp(argClass->name, param->coeur->_type->name))
											{
												if(argClass->superClass == NIL(t_class))
												{
													setError(PARAM_ERROR_2);
													toReturn = FALSE;
													break;
												}
												else argClass = argClass->superClass;
											}
										}
										break;
									case E_CALL_METHOD :
										
										break;
								}
								if(!toReturn)break;
								param = param->next;
								
							}while(getChild(entry, 1) != NIL(Tree));
						}
					}
				}

				/*if(toReturn == FALSE){
					
				}
				else{
					if(getChild(tree,2) == NULL){printf("meh\n");}
					entry = getChild(tree,2)->u.lvar;
					VarDeclP PDecl = decl->parametres;
					while(entry != NIL(VarDecl) && PDecl !=NIL(VarDecl)){

						toReturn = toReturn && ((strcmp(PDecl->coeur->_type->name,entry->coeur->_type->name) == 0) ? TRUE : FALSE);
						if(toReturn == FALSE){
							setError(PARAM_ERROR_2);
						}
						entry = entry->next;
						PDecl = PDecl->next; 
					}
				}*/
			}
			else if(tree->op == INST){
				
				t_method* constructor = getChild(tree,0)->u.lvar->coeur->_type->constructor;
				if(strcmp(constructor->name,getChild(tree,0)->u.lvar->name) == 0){

					
					VarDeclP entry = getChild(tree,1)->u.lvar;
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

					

						entry = getChild(tree,1)->u.lvar;
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
				else{
					setError(NO_EXISTING_METHOD);
					return FALSE;
				} 
			}
			if(getChild(tree,i) != NIL(Tree))verificationParametres(getChild(tree,i));
				
		}
		return toReturn;
}

/* Verification contextuelle : Super
 * 		Verification des boucles d'heritage
 * */
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

/* Verification contextuelle : Typage
 * 		On verifie que le nom n'existe pas 2 fois ou plus
 * */
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
				printf("%s : Nom objet deja existant\n", name);
				return FALSE;
			}else{
				DejaVu = 1;
			}
		}
		temp2 = temp2->next;
	}

	return TRUE;
}

/* Verification contextuelle : Typage
 * 		On verifie que le nom n'existe pas 2 fois ou plus
 * */
bool verificationNomVarDecl(VarDeclP env, char* name){
	
	int DejaVu = 0;
	/* On verifie que l'on ne donne pas a sa decl les noms
	* this, super ou result qui sont reserves */
	if(strcmp(name, "this") == 0)
	{
		printf("Interdiction d'utiliser le nom this (reserve)\n");
		return FALSE;
	}
	else if(strcmp(name, "super") == 0)
	{
		printf("Interdiction d'utiliser le nom super (reserve)\n");
		return FALSE;
	}
	else if(strcmp(name, "result") == 0)
	{
		printf("Interdiction d'utiliser le nom result (reserve)\n");
		return FALSE;
	}
	
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

/* Verification contextuelle : Typage
 * 		On verifie que le nom n'existe pas 2 fois ou plus (dans une meme classe/objet)
 * */
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

/* Verification contextuelle : Portee
 * 		On ajoute this, result et super a la liste des variables visibles. (ou pas cela depend des cas) pour les CLASSES
 * */
VarDeclP InitialisationSuperThisResultC(t_method* method, t_class* class, VarDeclP param){
	
	/**		CREATION DE this	*/
	VarDeclP this = NEW(1, VarDecl);
	this->coeur = NEW(1, t_variable);
	this->coeur->_type = class;
	this->name = "this";
	
	/**		CREATION DE result	*/
	VarDeclP result = NIL(VarDecl);
	if(method != NIL(t_method)){ /* On a method == NIL pour l'expression des attributs d'une classe/objet. */
		if(0 != strcmp(method->returnType->name,"Void")){
			result = NEW(1, VarDecl);
			result->coeur = NEW(1, t_variable);
			result->coeur->_type = method->returnType;
			result->name = "result";
		}
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

/* Verification contextuelle : Portee
 * 		On ajoute this, result et super a la liste des variables visibles. (ou pas cela depend des cas) pour les OBJETS
 * */
VarDeclP InitialisationSuperThisResultO(t_method* method, t_object* object, VarDeclP param){
	
	/**		CREATION DE this	*/
	VarDeclP this = NEW(1, VarDecl);
	this->coeur = NEW(1, t_variable);
	this->coeur->_obj = object;
	this->name = "this";
	
	/**		CREATION DE result	*/
	VarDeclP result = NIL(VarDecl);
	if(method != NIL(t_method)){
		if(0 != strcmp(method->returnType->name,"Void")){
			result = NEW(1, VarDecl);
			result->coeur = NEW(1, t_variable);
			result->coeur->_type = method->returnType;
			result->name = "result";
		}
	}

	if(result != NIL(VarDecl)){
		this->next = result;
		result->next = param;
	}else{
		this->next = param;
	}
	
	return this;
}

/* Utilitaire.
 * 		On affche une liste de VarDeclP.
 * */
void afficheListVarDeclP(VarDeclP liste){
	printf("Variables\n");
	while(liste != NIL(VarDecl)){
		if(liste->coeur->_type != NIL(t_class)) printf("%s(%s)..",liste->name,liste->coeur->_type->name);
		else printf("%s(%s)..",liste->name,liste->coeur->_obj->name);

		liste = liste->next;
	}
	printf("\n");
}

/* Verification contextuelle : Typage
 * 		On va chercher le type de retour d'une methode en fonction de la classe ou elle est, et son nom. Pour les CLASSES
 * */
t_class* getReturnC(t_class* class, char* nom_methode, list_ClassObjP env){
	t_method* i = class->methods;
	while(i != NIL(t_method)){
		if(!strcmp(i->name,nom_methode)) return FindClass(env->listClass, i->returnType->name);
		i = i->next;
	}
	return NIL(t_class); /* normalement n'arrive jamais.*/
}

/* Verification contextuelle : Typage
 * 		On va chercher le type de retour d'une methode en fonction de la classe ou elle est, et son nom. Pour les OBJETS
 * */
t_class* getReturnO(t_object* object, char* nom_methode, list_ClassObjP env){
	t_method* i = object->methods;
	while(i != NIL(t_method)){
		if(!strcmp(i->name,nom_methode)) return FindClass(env->listClass, i->returnType->name);
		i = i->next;
	}
	return NIL(t_class); /* normalement n'arrive jamais.*/
}

/* Verification contextuelle : Portee
 * 		On verifie que la super-classe d'une classe cible existe bel et bien.
 * */
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

/* Verification contextuelle : Typage
 * 		On verifie qu'une variable a bien ete initialisee. Utilisee pour "result".
 * */
bool isDeclared(TreeP tree, char* nameVar){
	int i;
	
	if(tree == NIL(Tree)) return FALSE;
	
	switch(tree->op){
		case I_BLOC:
			return isDeclared(getChild(tree, 1), nameVar);
		
		case LIST_INST:		
			for(i=0 ; i<tree->nbChildren ; i++){
				if(isDeclared(getChild(tree, i), nameVar)) return TRUE;
			}
			break;
			
		case I_AFF:
			if(getChild(tree, 0)->op == _ID){
				if(!strcmp(getChild(tree, 0)->u.lvar->name,nameVar)) return TRUE;
			}
			break;
			
		case I_ITE:
			return isDeclared(getChild(tree, 1), nameVar) && isDeclared(getChild(tree, 2), nameVar);
			
		default:
			break;
	}
	
	return FALSE;
	
}
