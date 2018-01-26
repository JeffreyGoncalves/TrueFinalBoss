#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "tp.h"


t_method* DMtoS(TreeP Tree,t_class* listClass);
t_class* FindClass(t_class* listClass, char* str);
t_class* makeListClass(TreeP TreeClass);