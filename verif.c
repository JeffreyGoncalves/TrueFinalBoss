#include "verif.h"

extern char *strdup(const char*);

extern void setError(int code);

/***************** Verifications contextuelles liees a la portee *****************/
/*NB : FAUDRAIT-IL VERIFIER UNIQUEMENT LA PORTEE SI ELLE EST FAUSSE OU LA PORTEE ET LE TYPE MEME SI LE PREMIER SE REVELE FAUX ? */
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
		 
		 /* TreeP tree : représente le corps du programme.
		  * list_ClassObjP classObjList : représente l'ensemble des déclarations des objets et classes.
		  */
		  
		/* Verification partie classe/objet puis
		 * Verification du bloc principal (portee) */
		bool isCorrect = verifPorteeClassObj(classObjList)
		  && verifPorteeBloc(tree, NULL, classObjList->listObj);
		 
		/*A DISCUTER */ 
		if(!isCorrect)
			abort();
	}
}

bool verifPorteeMeth(TreeP tree, t_class *class)
{
	return FALSE;
}

bool verifPorteeInst(TreeP inst, VarDeclP listDecl, t_object *listObj, short op)
{
	bool toReturn;
	
	if(op == I_BLOC) /* Instruction dans le bloc principal ou 
						un sous bloc du bloc principal */ 
	{
		/* Bloc */
		if(inst->op == I_BLOC)
			toReturn = verifPorteeBloc(inst, listDecl, listObj);
			
		/* Return, non autorise ici */
		else if(inst->op == I_RETURN)
		{
			setError(RETURN_ERROR);
			toReturn = FALSE;
		}
		
		/* ITE */
		else if(inst->op == I_ITE)
		{
			TreeP Expr = getChild(inst, 0),
				  Inst1 = getChild(inst, 1),
				  Inst2 = getChild(inst, 2);
			toReturn = verifPorteeExpr(Expr, listDecl, listObj, I_BLOC)
					&& verifPorteeInst(Inst1, listDecl, listObj, I_BLOC) 
					&& verifPorteeInst(Inst2, listDecl, listObj, I_BLOC);
		}
		
		/* Affectation */
		else if(inst->op == I_AFF)
		{
			TreeP Obj = getChild(inst, 0),
				  Expr = getChild(inst, 1);
			toReturn = verifPorteeExpr(Obj, listDecl, listObj, I_BLOC)
					&& verifPorteeExpr(Expr, listDecl, listObj, I_BLOC);
		}
		
		/* Expression */
		else if(inst->op == I_EXPRRELOP)
		{
			toReturn = verifPorteeExpr(getChild(inst, 0), listDecl, listObj, I_BLOC);
		}  
		
		else
			toReturn = FALSE;
			/* CAS NE DEVANT JAMAIS ARRIVER */
	}
	
	else if(op == VAR_DEF_METH)/* Bloc de Methode */
	{
		/* A CONTINUER */
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
	bool toReturn = TRUE;
	t_object* objectBuffer = classObjList->listObj;
	
	while(objectBuffer != NIL(t_object)){
		if(!verifPorteeObject(objectBuffer, classObjList) && toReturn) toReturn = FALSE;
		objectBuffer = objectBuffer->next;
	}
	
	return toReturn;
}

bool verifPorteeObject(t_object* object, list_ClassObjP classObjList)
{
	bool toReturn = TRUE;
	
	/*	On vérifie que le nom n'existe pas deja */
	toReturn = toReturn && verificationNomClasse(classObjList, object->name);
	
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
	
	/* Verification methodes */
	t_method* methodBuffer = object->methods;
	while(methodBuffer != NIL(t_method))
	{
		if((!verificationNomMethod(object->methods, methodBuffer->name) || !verifPorteeMethodO(methodBuffer, object, classObjList))
		 && toReturn) 
		{
			 toReturn = FALSE;
		}
		methodBuffer = methodBuffer->next;
	}
	
	return toReturn;
}

bool verifPorteeListClass(list_ClassObjP classObjList)
{
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
	
	/*	On vérifie que le nom n'existe pas deja */
	if(!verificationNomClasse(classObjList, class->name) && toReturn) toReturn = FALSE;
	
	/*	Verification de la super-classe */
	t_class* ClassBuffer = FindClass(classObjList->listClass, class->superClass->name);
	if(ClassBuffer == NIL(t_class)) 
	{
		setError(CLASS_NOT_FOUND);
		toReturn = FALSE;
	}
	free(class->superClass); /** C'était une classe temporaire.*/
	class->superClass = ClassBuffer;
	
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
	
	/* Verification methodes */
	t_method* methodBuffer = class->methods;
	while(methodBuffer != NIL(t_method)){
		if(!verificationNomMethod(class->methods, methodBuffer->name) && toReturn) toReturn = FALSE;
		if(!verifPorteeMethodC(methodBuffer, class, classObjList) && toReturn) toReturn = FALSE;
		
		methodBuffer = methodBuffer->next;
	}
	
	/* Verification constructeur */
	methodBuffer = class->constructor;
	if(!verifPorteeConstructor(methodBuffer, class, classObjList) && toReturn) toReturn = FALSE;
	
	return toReturn;
}

/*
typedef struct t_method{
	char* name;
	t_class* returnType;
	short nbParametres ;
	VarDeclP parametres;
    TreeP bloc;
	int isRedef;
	struct t_method* next;
}t_method;
*/

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
	
	/* Verification du type de retour */
	ClassBuffer = FindClass(classObjList->listClass, method->returnType->name);
	if(ClassBuffer == NIL(t_class))
	{
		setError(CLASS_NOT_FOUND);
		toReturn = FALSE;
	}
	free(method->returnType); /** C'était une classe temporaire.*/
	method->returnType = ClassBuffer;
	
	
	
	/*if(!verifPorteeBloc(TreeP tree, VarDeclP listDecl, t_object *listObj)) return FALSE;*/
	
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
	
	/* Verification du type de retour */
	ClassBuffer = FindClass(classObjList->listClass, method->returnType->name);
	if(ClassBuffer == NIL(t_class))
	{
		setError(CLASS_NOT_FOUND);
		toReturn = FALSE;
	}
	free(method->returnType); /** C'était une classe temporaire.*/
	method->returnType = ClassBuffer;
	
	
	
	/*if(!verifPorteeBloc(TreeP tree, VarDeclP listDecl, t_object *listObj)) return FALSE;*/
	
	return toReturn;
}

bool verifPorteeConstructor(t_method* method, t_class* class, list_ClassObjP classObjList){
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
		free(VarDeclBuffer->coeur->_type); /** C'était une classe temporaire.*/
		VarDeclBuffer->coeur->_type = ClassBuffer;
		
		VarDeclBuffer = VarDeclBuffer->next;
	}
	
	/* le type de retour est déjà vérifié de par la génération des structures. */
	
	/*  Idem pour le nom. */
	
	/*if(!verifPorteeBloc(TreeP tree, VarDeclP listDecl, t_object *listObj)) return FALSE;*/
	
	return toReturn;
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

/* A TRAVAILLER */
bool verifPorteeExpr(TreeP Expr, VarDeclP listDecl, t_object *listObj, short op)
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
			t_object *objSel = listObj;
			while(objSel != NULL)
			{
				if(!strcmp(objSel->name, Expr->u.lvar->name))
				{
					Expr->u.lvar->coeur->_obj = objSel;
				}
				varSel = varSel->next;
			}
			
			/* Aucun objet n'a ete trouve */
			setError(VAR_NOT_FOUND);
			toReturn = FALSE;
		}
		else return FALSE;
	}
	
	else
	{
		bool toReturn = TRUE;
		for(i=0;i<Expr->nbChildren;i++)
			toReturn = toReturn && verifPorteeExpr(Expr, listDecl, listObj, op);
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
