#include "verif.h"

extern char *strdup(const char*);

extern void setError(int code);

bool verifPorteeExpr(TreeP Expr, VarDeclP listDecl, t_object *listObj, short op);
bool verifPorteeInst(TreeP inst, VarDeclP listDecl, t_object *listObj, short op);
bool verifPorteeMeth(TreeP tree, t_class *class);
bool verifPorteeBloc(TreeP tree, VarDeclP listDecl, t_object *listObj);
bool verifPorteeClassObj(TreeP classObj, list_ClassObjP classObjList);
bool verifPorteeObject(TreeP tree, list_ClassObjP classObjList);
bool verifPorteeClass(TreeP tree, list_ClassObjP classObjList);

/***************** Verifications contextuelles liees a la portee *****************/
void verifPorteeProg(TreeP tree, list_ClassObjP classObjList)
{
	if(tree != NULL && tree->op == PROG)
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
		
		/* Separation de la partie classe/objet et de la partie bloc principal */
		TreeP classObj = getChild(tree, 0);
		TreeP principalBlock = getChild(tree, 1);
		
		/* Verification du bloc principal (portee) */
		verifPorteeBloc(principalBlock, NULL, classObjList->listObj);
		
		/* Verification partie classe/objet */
		verifPorteeClassObj(classObj, classObjList);
	}
}

bool verifPorteeMeth(TreeP tree, t_class *class)
{
	return FALSE;
}

bool verifPorteeInst(TreeP inst, VarDeclP listDecl, t_object *listObj, short op)
{
	if(op == I_BLOC) /* Instruction dans le bloc principal ou 
						un sous bloc du bloc principal */ 
	{
		/* Bloc */
		if(inst->op == I_BLOC)
			return verifPorteeBloc(inst, listDecl, listObj);
			
		/* Return, non autorise ici */
		if(inst->op == I_RETURN)
		{
			setError(RETURN_ERROR);
			return FALSE;
		}
		
		/* ITE */
		if(inst->op == I_ITE)
		{
			TreeP Expr = getChild(inst, 0),
				  Inst1 = getChild(inst, 1),
				  Inst2 = getChild(inst, 2);
			return (verifPorteeExpr(Expr, listDecl, listObj, I_BLOC) &&
					verifPorteeInst(Inst1, listDecl, listObj, I_BLOC) &&
					verifPorteeInst(Inst2, listDecl, listObj, I_BLOC));
		}
		
		/* Affectation */
		if(inst->op == I_AFF)
		{
			TreeP Obj = getChild(inst, 0),
				  Expr = getChild(inst, 1);
			return (verifPorteeExpr(Obj, listDecl, listObj, I_BLOC) &&
					verifPorteeExpr(Expr, listDecl, listObj, I_BLOC));
		}
		
		/* Expression */
		if(inst->op == I_EXPRRELOP)
		{
			return verifPorteeExpr(getChild(inst, 0), listDecl, listObj, I_BLOC);
		} 
	}
	
	else if(op == VAR_DEF_METH)/* Bloc de Methode */
	{
		
	}
	return FALSE;
		
}

bool verifPorteeClassObj(TreeP classObj, list_ClassObjP classObjList)
{
	bool toReturn = TRUE;
	
	while(classObj != NIL(Tree))
	{
		TreeP listClassObj = getChild(classObj, 0);
		TreeP classObjTree = getChild(classObj, 1);
		
		/* On regarde ce qu'est classObjTree : classe ou objet */
		/* Cas classe */
		if(classObjTree->op == CLAS)
			toReturn = toReturn && verifPorteeClass(getChild(classObjTree, 0), classObjList);
		/* Cas Objet */
		if(classObjTree->op == OBJ)
			toReturn = toReturn && verifPorteeObject(getChild(classObjTree, 0), classObjList);
		
		toReturn = toReturn && verifPorteeClassObj(listClassObj, classObjList);
	}
	return toReturn;
	
}

bool verifPorteeObject(TreeP tree, list_ClassObjP classObjList)
{
	char *objName = getChild(tree, 0)->u.str;
	t_object *actualObj = NIL(t_object);
	bool toReturn = TRUE;
	tree = getChild(tree, 1);
	
	/* On cherche son enregistrement dans classListObj (actualObj) dans l'optique de la recuperation
	 * des champs (champsObj) */
	t_object *listObj = classObjList->listObj;
	while(listObj != NIL(t_object))
	{
		if(!strcmp(listObj->name, objName))
		{
			actualObj = listObj;
			break;
		}
		else listObj = listObj->next;
	}
	VarDeclP champsObj = actualObj->attributes;
	
	/* On a fini et on s'attaque aux bloc des methodes */
	while(tree != NIL(Tree))
	{
		TreeP methVar = getChild(tree, 0);
		TreeP next = getChild(tree, 1);
		
		/* Traitement des blocs des methodes */
		if(methVar->op == VAR_DEF_METH)
		{
			TreeP methodType = getChild(methVar, 0);
			/* Il faut ici differencier les deux cas : 
			* Override DEF ID'(' ListParamClause ')' ':' ID AFF ExprRelop et
			* Override DEF ID'(' ListParamClause ')' ClassClause IS block 
			* De plus, OVERRIDE n'a aucun sens vu qu'ici on considere un objet
			* qui ne peut donc pas heriter */
			if(methodType->op == DECL_METH_1)
			{
				if(getChild(methodType, 2) != NIL(Tree))
				{
					setError(OVERRIDE_ERROR);
					return FALSE;
					/* Erreur car OVERRIDE interdit */
				}
				else
				{
					
				}				
			}
			else if(methodType->op == DECL_METH_2)
			{
				if(getChild(methodType, 1) != NIL(Tree))
				{
					setError(OVERRIDE_ERROR);
					return FALSE;
					/* Erreur car OVERRIDE interdit */
				}
				else
				{
					/* Recuperer la VarDeclP des parametres 
					 * puis ajouter le VarDeclP des champs
					 * et enfin tester le bloc*/
					VarDeclP listParam = NIL(VarDecl), returnType = NIL(VarDecl);
					if(getChild(methodType, 2) != NIL(Tree))
						listParam = getChild(methodType, 2)->u.lvar;
					if(getChild(methodType, 3) != NIL(Tree))
						returnType = getChild(methodType, 3)->u.lvar;	/* VarDeclP isole avec seulement le
																			nom de la classe de retour */
					if(returnType != NIL(VarDecl))	/* On cherche dans la liste des classes le type de retour */
					{
						t_class* classSel = classObjList->listClass;
						bool isClassFound = FALSE;
						while(classSel != NIL(t_class))
						{
							if(!strcmp(classSel->name, returnType->name))	/* On trouve la classe */
							{
								free(returnType);
								returnType = NEW(1, VarDecl);
								/* Lier par pointage la class selectionne avec returnType */
							}
						}
						
					}
				}
			}
		}
		
		tree = next;
	}
	return toReturn;
}

bool verifPorteeClass(TreeP tree, list_ClassObjP classObjList)
{
	return FALSE;
}

bool verifPorteeBloc(TreeP tree, VarDeclP listDecl, t_object *listObj)
{
	bool toReturn = TRUE;
	VarDeclP listVarDecl = getChild(tree, 0)->u.lvar;
	TreeP listInst = getChild(tree,1);
	
	/**** Verification de la liste de declaration ****/
	VarDeclP varSel = listVarDecl;
	int i=0, j=0, bufferSize = 10;
	char **names = malloc(bufferSize*sizeof(char*));
	for(i=0;varSel != NULL;i++)
	{
		/*Cas ou le buffer de noms n'est pas assez grand*/
		if(i+1 > bufferSize)
		{
			bufferSize *= 2;
			char **newName = realloc(names, bufferSize*sizeof(char*));
			if(newName != NULL)names = newName;
		}
		
		/*On compare les noms*/
		names[i] = varSel->name;
		for(j=0;j<i;j++)
		{
			if(!strcmp(names[i], names[j]))
			{
				setError(REDECL_ERROR);
				toReturn = FALSE;
			}
			else if(!strcmp(names[i], "this") || !strcmp(names[i], "super") || !strcmp(names[i], "result"))
			{
				setError(RESERV_DECL_ERROR);
				toReturn = FALSE;
			}
		}
		/*Passage a la variable suivante*/
		varSel = varSel->next;
	}
	/*************************************************/
	
	/****	Ajout des declarations precedentes non prioritaires	******/
	/* Note : cela permet de gerer le masquage (les variables 
	 * du bloc sont prioritaires sur les precedentes */
	if(listDecl != NIL(VarDecl))
	{
		VarDeclP varSel = listVarDecl;
		while(varSel->next != NULL)
			varSel = varSel->next;
		varSel->next = listDecl;
	}
	/*****************************************************************/
	
	/*** Verification des instructions (portee) ***/
	if(listInst != NULL)
	{
		while(listInst->nbChildren == 2)
		{
			toReturn = toReturn && verifPorteeInst(getChild(listInst, 0), listVarDecl, listObj, I_BLOC);
			listInst = getChild(listInst, 1);
		}
		toReturn = toReturn && verifPorteeInst(getChild(listInst, 0), listVarDecl, listObj, I_BLOC);
	}
	/**********************************************/
	
	return toReturn;
}

bool verifPorteeExpr(TreeP Expr, VarDeclP listDecl, t_object *listObj, short op)
{
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
					return TRUE;
				}
				varSel = varSel->next;
			}
			
			/* Objets isoles */
			t_object *objSel = listObj;
			while(objSel != NULL)
			{
				if(!strcmp(objSel->name, Expr->u.lvar->name))
				{
					Expr->u.lvar->coeur->_obj = objSel;
					return TRUE;
				}
				varSel = varSel->next;
			}
			return FALSE;
		}
		else return FALSE;
	}
	
	else
	{
		bool toReturn = TRUE;
		for(i=0;i<Expr->nbChildren;i++)
			toReturn = toReturn && verifPorteeExpr(Expr, listDecl, listObj, op);
			
		return toReturn;
	}
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
			
		case EXPR_RELOP:
			if(0 == strcmp(veriFils[0].type.class->name,"Interger") && 0 == strcmp(veriFils[1].type.class->name,"Interger")){
				result.type.class = veriFils[0].type.class;
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
			result.type.class = FindClass(env->listClass, noeud->u.str);/*TODO*/
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
		Vtypage okko /*= getReturnType(method->bloc)*/;
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
		Vtypage okko /*= getReturnType(method->bloc)*/;
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
				/*VERIF NB PARAMETRES*/
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

				/*VERIF TYPE PARAMETRES*/

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

					/*VERIF NB PARAMETRES*/
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

					/*VERIF TYPE PARAMETRES*/

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
}
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

bool verificationNomClasse(list_ClassObjP env, t_class* class){

	t_class* temp = env->listClass;

	if('A' >= class->name[0] || class->name[0] >= 'Z'){

		printf("%s : Nom de classe sans majuscule !\n", class->name);
		return FALSE;
	}

	while(temp != NIL(t_class) || temp->next != NIL(t_class)){

		if(strcmp(temp->name,class->name) == 0){

			printf("%s : Nom de classe deja existant\n",class->name);
			return FALSE;
		}
	}

	return TRUE;
}



