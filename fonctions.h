#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "tp.h"
#include "tp_y.h"

t_method* DMtoS(TreeP Tree, t_class* firstClass);
t_method* makeConstructor(t_class* class, VarDeclP param, TreeP corps);
VarDeclP giveAllAttributes(TreeP tree, t_class* firstClass);
t_method* giveAllMethod(TreeP tree, t_class* firstClass);
t_class* FindClass(t_class* listClass, char* str);
t_class* makeListClass(TreeP TreeClass, t_class* firstClass);
