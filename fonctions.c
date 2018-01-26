#include "fonctions.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "tp.h"
#include "tp_y.h"

extern int yyparse();
extern int yylineno;

/*typedef struct t_class{
	char* name;
	struct t_method** constructor;
	struct t_method** methods;
	struct VarDecl* attributes;
	struct t_class* superClass;
}t_class;*/

/* REMPLISSAGE STRUCT DE CLASSE */
t_class* makeListClass(TreeP TreeClass){
	
	if(List != NIL(Tree)){
		myClass = NEW(1, t_class);
		
		/*LE NOM*/
		myClass->name = getChild(TreeClass, 0)->u.str;
		
		/*LA LISTE de PARAMETRES*/
		
	}else{
		return NIL(t_class);
	}
}
