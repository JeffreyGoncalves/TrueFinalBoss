#include "verif.h"

extern char *strdup(const char*);

extern void setError(int code);

bool verifPorteeExpr(TreeP Expr, VarDeclP listDecl, t_object *listObj, short op);
bool verifPorteeInst(TreeP inst, VarDeclP listDecl, t_object *listObj, short op);
bool verifPorteeMeth(TreeP tree, t_class *class);
bool verifPorteeBloc(TreeP tree, VarDeclP listDecl, t_object *listObj);
bool verifPorteeClassObj(TreeP classObj, list_ClassObjP classObjList);

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
	}
	return FALSE;
		
}

bool verifPorteeClassObj(TreeP classObj, list_ClassObjP classObjList)
{
	return FALSE;
}

bool verifPorteeBloc(TreeP tree, VarDeclP listDecl, t_object *listObj)
{
	bool toReturn = TRUE;
	VarDeclP listVarDecl = listDecl;
	TreeP listInst = tree->u.children[1];
	
	/* On ajoute au bloc selectionne les declaration precedentes */
	if(listDecl == NULL)listVarDecl = (VarDeclP)tree->u.children[0];
	else
	{
		VarDeclP varSel = listDecl;
		while(varSel->next != NULL)
			varSel = varSel->next;
		varSel->next = (VarDeclP)tree->u.children[0];
	}
	
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
	return FALSE;
}
/******************************************************************************************/

Vtypage verifcationTypageNoeud(TreeP noeud, list_ClassObjP env){
	
	Vtypage result;
	Vtypage veriFils[5];
	int i;
	result.succes = 1;
	
	if(noeud == NIL(Tree)) return result;
	
	switch(noeud->op){
		
		case I_BLOC:
			return verifTypageSuccesFils(noeud->nbChildren, noeud, env);
		
		case DECL:
			return verifcationTypageListVarDecl(noeud->u.lvar, env);
		case LIST_PARAM:
			return verifcationTypageListVarDecl(noeud->u.lvar, env);
			
		case INST:
			return verifTypageSuccesFils(noeud->nbChildren, noeud, env);

		case I_RETURN: /* PAS FINI ! */
			return result;
		
		case I_EXPRRELOP:
			return verifTypageSuccesFils(noeud->nbChildren, noeud, env);
			
		case I_ITE:
			for(i=0 ; i<noeud->nbChildren ; i++){
				veriFils[i] = verifcationTypageNoeud(getChild(noeud, i), env);
				if(!(veriFils[i].succes)){
					result.succes = 0;
					return result;
				}
			}

			if(0 == strcmp(veriFils[0].class->name,"Interger")){
				return result;
			}
			break;
			
		case I_AFF:
			for(i=0 ; i<noeud->nbChildren ; i++){
				veriFils[i] = verifcationTypageNoeud(getChild(noeud, i), env);
				if(!(veriFils[i].succes)){
					result.succes = 0;
					return result;
				}
			}

			if(0 == strcmp(veriFils[0].class->name, veriFils[1].class->name)){
				return result;
			}
			break;
			
		case EXPR_RELOP:
			for(i=0 ; i<noeud->nbChildren ; i++){
				veriFils[i] = verifcationTypageNoeud(getChild(noeud, i), env);
				if(!(veriFils[i].succes)){
					result.succes = 0;
					return result;
				}
			}

			if(0 == strcmp(veriFils[0].class->name,"Interger") && 0 == strcmp(veriFils[1].class->name,"Interger")){
				result.class = veriFils[0].class;
				return result;
			}
			break;
			
		case SUM:
			for(i=0 ; i<noeud->nbChildren ; i++){
				veriFils[i] = verifcationTypageNoeud(getChild(noeud, i), env);
				if(!(veriFils[i].succes)){
					result.succes = 0;
					return result;
				}
			}

			if(0 == strcmp(veriFils[0].class->name,"Interger") && 0 == strcmp(veriFils[1].class->name,"Interger")){
				result.class = veriFils[0].class;
				return result;
			}
			break;
			
		case MIN:
			for(i=0 ; i<noeud->nbChildren ; i++){
				veriFils[i] = verifcationTypageNoeud(getChild(noeud, i), env);
				if(!(veriFils[i].succes)){
					result.succes = 0;
					return result;
				}
			}

			if(0 == strcmp(veriFils[0].class->name,"Interger") && 0 == strcmp(veriFils[1].class->name,"Interger")){
				result.class = veriFils[0].class;
				return result;
			}
			break;
		
		case MULT:
			for(i=0 ; i<noeud->nbChildren ; i++){
				veriFils[i] = verifcationTypageNoeud(getChild(noeud, i), env);
				if(!(veriFils[i].succes)){
					result.succes = 0;
					return result;
				}
			}

			if(0 == strcmp(veriFils[0].class->name,"Interger") && 0 == strcmp(veriFils[1].class->name,"Interger")){
				result.class = veriFils[0].class;
				return result;
			}
			break;
		
		case DIVI:
			for(i=0 ; i<noeud->nbChildren ; i++){
				veriFils[i] = verifcationTypageNoeud(getChild(noeud, i), env);
				if(!(veriFils[i].succes)){
					result.succes = 0;
					return result;
				}
			}

			if(0 == strcmp(veriFils[0].class->name,"Interger") && 0 == strcmp(veriFils[1].class->name,"Interger")){
				result.class = veriFils[0].class;
				return result;
			}
			break;
			
		case AND:
			for(i=0 ; i<noeud->nbChildren ; i++){
				veriFils[i] = verifcationTypageNoeud(getChild(noeud, i), env);
				if(!(veriFils[i].succes)){
					result.succes = 0;
					return result;
				}
			}

			if(0 == strcmp(veriFils[0].class->name,"String") && 0 == strcmp(veriFils[1].class->name,"String")){
				result.class = veriFils[0].class;
				return result;
			}
			break;
			
		case CST:
			result.class = FindClass(env->listClass, "Integer");
			return result;
			break;
		
		case _ID:
			result.class = FindClass(env->listClass, "String");
			return result;
			break;
			
		case CAST:
			for(i=0 ; i<noeud->nbChildren ; i++){
				veriFils[i] = verifcationTypageNoeud(getChild(noeud, i), env);
				if(!(veriFils[i].succes)){
					result.succes = 0;
					return result;
				}
			}

			if(0 == strcmp(veriFils[0].class->name,"String") && 0 == strcmp(veriFils[1].class->name,"String")){
				return result;
			}
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
		
		if(!(expr.succes) || 0 != strcmp(expr.class->name, liste->coeur->_type->name)){
			result.succes = 0;
			return result;
		}
			
		liste = liste->next;
	}
	result.succes = 1;
	return result;
	
}

bool verificationNbParametres(t_method* method, VarDeclP entry){

	VarDeclP tmp = entry;
	int givenNb = 0;

	while(tmp != NIL(VarDecl) || tmp->next != NIL(VarDecl)){
		givenNb++;
		tmp = tmp->next;
	}

	return (method->nbParametres == givenNb) ? TRUE : FALSE;
}

bool verificationBoucleHeritage(t_class* listClasses, t_class class){
	return TRUE;
}

bool verificationNomClasse(t_class* listClasses, t_class class){

	t_class* temp = listClasses;

	if('A' >= class.name[0] || class.name[0] >= 'Z'){

		printf("%s : Nom de classe sans majuscule !\n", class.name);
		return FALSE;
	}

	while(temp != NIL(t_class) || temp->next != NIL(t_class)){

		if(strcmp(temp->name,class.name) == 0){

			printf("%s : Nom de classe deja existant\n",class.name);
			return FALSE;
		}
	}

	return TRUE;
}
