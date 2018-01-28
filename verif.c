#include <string.h>
#include <stdio.h>
#include "tp.h"
#include "tp_y.h"

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
