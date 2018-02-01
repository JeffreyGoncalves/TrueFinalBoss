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
bool verificationNomClasse(list_ClassObjP env, t_class* class);
