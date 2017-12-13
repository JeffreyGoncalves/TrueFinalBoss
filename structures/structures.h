#include <stdlib.h>

typedef struct t_affect {
	t_variable variable;
	t_expr valeur;
}

typedef struct t_class {
	char* name;
	t_method constructeur
	t_method* methods;
	t_varIdent* attributes;
	t_class superClass;
	
}


typedef union t_expr {

};

typedef union t_instr {
	t_affect affectation;
	t_instr* bloc;
	t_return _return;
	t_ite _ite;
}

typedef struct t_variable {
	t_varIdent ident;
	t_value value;
	
}

typedef struct t_varIdent {
	char* name;
	t_class _type;

}

typedef struct t_method {
	char* name;
	t_type returnType;
	t_varIdent* parametres;
	t_instr* instructions;
	bool isRedef;
	
}
