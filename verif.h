#ifndef VERIF_H
#define VERIF_H

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "fonctions.h"
#include "tp.h"

typedef struct Vtypage{
	union{
		VarDeclP variable;
		t_method* method;
		t_class* class;
		t_object* object;
	}type;
	int succes;
}Vtypage, *VtypageP;

bool verificationTypage(list_ClassObjP env, TreeP core);
bool verificationTypageEnvironnement(list_ClassObjP env);
Vtypage verifcationTypageListVarDecl(VarDeclP liste, list_ClassObjP env);
Vtypage verifcationTypageNoeud(TreeP noeud, list_ClassObjP env);
Vtypage verifTypageSuccesFils(short nbre, TreeP noeud, list_ClassObjP env);
int AEstSuperDeB(char* A, char* B,list_ClassObjP env);
bool verificationTypageMethode(t_class* C, t_method* method, list_ClassObjP env);
bool verificationTypageMethodeO(t_method* method, list_ClassObjP env);
t_class* getReturnType(TreeP tree, list_ClassObjP env);
bool isDeclared(TreeP tree, char* nameVar);

bool verificationParametres(TreeP block);
bool verificationBoucleHeritage(t_class* class);
bool verificationNomClasse(list_ClassObjP env, char* name);
bool verificationNomMethod(t_method* env, char* name);
bool verificationNomVarDecl(VarDeclP env, char* name);

bool verifPorteeExpr(TreeP Expr, VarDeclP listDecl, list_ClassObjP classObjList);
bool verifPorteeInst(TreeP inst, VarDeclP listDecl, list_ClassObjP classObjList);
bool verifPorteeMeth(TreeP tree, t_class *class);
bool verifPorteeBloc(TreeP tree, VarDeclP listDecl, list_ClassObjP classObjList);
bool verifPorteeClassObj(list_ClassObjP classObjList);
bool verifPorteeListObject(list_ClassObjP classObjList);
bool verifPorteeObject(t_object* object, list_ClassObjP classObjList);
bool verifPorteeListClass(list_ClassObjP classObjList);
bool verifPorteeClass(t_class* class, list_ClassObjP classObjList);
bool verifPorteeMethodC(t_method* method, t_class* class, list_ClassObjP classObjList);
bool verifPorteeMethodO(t_method* method, t_object* object, list_ClassObjP classObjList);
bool verifPorteeConstructor(t_method* method, t_class* class, list_ClassObjP classObjList);
void verifPorteeProg(TreeP tree, list_ClassObjP classObjList);

VarDeclP InitialisationSuperThisResultC(t_method* method, t_class* class, VarDeclP param);
VarDeclP InitialisationSuperThisResultO(t_method* method, t_object* object, VarDeclP param);

void afficheListVarDeclP(VarDeclP liste);

t_class* getReturnC(t_class* class, char* nom_methode, list_ClassObjP env);
t_class* getReturnO(t_object* object, char* nom_methode, list_ClassObjP env);

bool verificationSuperClass(list_ClassObjP classObjList);
#endif
