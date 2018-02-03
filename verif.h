#ifndef VERIF_H
#define VERIF_H

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "fonctions.h"


typedef struct Vtypage{
	union{
		VarDeclP variable;
		t_method* method;
		t_class* class;
		t_object* object;
	}type;
	int succes;
}Vtypage, *VtypageP;

Vtypage verifcationTypageListVarDecl(VarDeclP liste, list_ClassObjP env);
Vtypage verifcationTypageNoeud(TreeP noeud, list_ClassObjP env);
Vtypage verifTypageSuccesFils(short nbre, TreeP noeud, list_ClassObjP env);
int AEstSuperDeB(char* A, char* B,list_ClassObjP env);
int verificationTypageMethode(t_class* C, t_method* method, list_ClassObjP env);
int verificationTypageMethodeO(t_method* method, list_ClassObjP env);
t_class* getReturnType(TreeP tree, list_ClassObjP env);

bool verificationParametres(TreeP block);
bool verificationBoucleHeritage(list_ClassObjP env, t_class* class);
bool verificationNomClasse(list_ClassObjP env, char* name);
bool verificationNomMethod(t_method* env, char* name);
bool verificationNomVarDecl(VarDeclP env, char* name);

bool verifPorteeExpr(TreeP Expr, VarDeclP listDecl, t_object *listObj, short op);
bool verifPorteeInst(TreeP inst, VarDeclP listDecl, t_object *listObj, short op);
bool verifPorteeMeth(TreeP tree, t_class *class);
bool verifPorteeBloc(TreeP tree, VarDeclP listDecl, t_object *listObj);
bool verifPorteeClassObj(list_ClassObjP classObjList);
bool verifPorteeListObject(list_ClassObjP classObjList);
bool verifPorteeObject(t_object* object, list_ClassObjP classObjList);
bool verifPorteeListClass(list_ClassObjP classObjList);
bool verifPorteeClass(t_class* class, list_ClassObjP classObjList);
bool verifPorteeMethodC(t_method* method, t_class* class, list_ClassObjP classObjList);
bool verifPorteeMethodO(t_method* method, t_object* object, list_ClassObjP classObjList);
bool verifPorteeConstructor(t_method* method, t_class* class, list_ClassObjP classObjList);
void verifPorteeProg(TreeP tree, list_ClassObjP classObjList);

#endif
