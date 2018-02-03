#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "tp.h"

void makeCode(TreeP tree, FILE* pFile);
int tailleAlloc(VarDeclP varDecl, int* taille);
int getOffset(TreeP attribut, int* offset);