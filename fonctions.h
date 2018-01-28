#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "tp.h"

typedef struct Vtypage{
	t_class* class;
	int succes;
}Vtypage, *VtypageP;

t_method* DMtoS(TreeP Tree, t_class* firstClass);
t_method* makeConstructor(t_class* class, VarDeclP param, TreeP corps);
VarDeclP giveAllAttributes(TreeP tree, t_class* firstClass);
t_method* giveAllMethod(TreeP tree, t_class* firstClass);
t_class* FindClass(t_class* listClass, t_class* str);
t_class* makeClass(TreeP TreeClass, t_class* firstClass);
list_ClassObjP makeListClassObj(TreeP TreeList);
t_object* makeObj(TreeP TreeObject, t_class* firstClass);
void afficheClass(t_class* liste);
void afficheParam(VarDeclP liste);
void afficheNomMethod(t_method* liste);
void compile(TreeP listClassObject, TreeP core);
