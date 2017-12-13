#include <stdlib.h>

typedef struct t_method {
	char* name;
	t_type returnType;
	t_varIdent* parametres;
	t_instr* instructions;
	bool isRedef;
	
}
