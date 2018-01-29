#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "fonctions.h"

typedef struct Vtypage{
	t_class* class;
	int succes;
}Vtypage, *VtypageP;

Vtypage verifcationTypageListVarDecl(VarDeclP liste, list_ClassObjP env);
Vtypage verifcationTypageNoeud(TreeP noeud, list_ClassObjP env);
Vtypage verifTypageSuccesFils(short nbre, TreeP noeud, list_ClassObjP env);
bool verificationNbParametres(t_method* method, VarDeclP entry);