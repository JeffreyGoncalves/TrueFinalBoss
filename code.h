#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "tp.h"

void makeCode(TreeP tree, FILE* pFile);
void makeCodeClasse(t_class* class, FILE* pFile);
void makeCodeObjet(t_object* obj, FILE* pFile);
int tailleAlloc(VarDeclP decl);
int getOffsetObj(t_object* obj, char* nom);
int getOffsetAttr(VarDeclP decl, char* nom);
void InitTV(list_ClassObjP env, FILE* pFile,t_method* list);
t_method* InitMethod(list_ClassObjP env, FILE* pFile);
void CallMethod(list_ClassObjP env, FILE* pFile,t_method* list);
int getOffsetMeth(t_method* meth, char* nom);
