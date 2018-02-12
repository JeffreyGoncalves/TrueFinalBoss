#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "tp.h"

void makeCode(TreeP tree,list_ClassObjP env,FILE* pFile);
void makeCodeClasse(t_class* class, FILE* pFile);
void makeCodeObjet(t_object* obj, FILE* pFile);
void makeCodeAffect(TreeP exprG, TreeP exprD, list_ClassObjP env, FILE* pFile);
int tailleAlloc(VarDeclP decl);
int getOffsetObj(t_object* obj, t_object* listObj);
int getOffsetAttr(VarDeclP decl, char* nom);
void InitTV(list_ClassObjP env, FILE* pFile,t_method* list);
t_method* InitMethod(list_ClassObjP env, FILE* pFile);
void CallMethod(list_ClassObjP env, FILE* pFile,t_method* list);
int getOffsetMeth(t_method* meth, char* nom);
void GcCallMethod(list_ClassObjP env,FILE* pFile,t_method* list,TreeP tree);
