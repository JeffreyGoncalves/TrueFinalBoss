#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "tp.h"

void makeCode(TreeP tree, FILE* pFile);
void makeCodeClasse(t_class* class, FILE* pFile);
void makeCodeObjet(t_object* obj, FILE* pFile);
int tailleAlloc(VarDeclP decl);
