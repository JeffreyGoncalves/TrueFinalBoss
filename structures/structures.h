#include <stdlib.h>

typedef struct t_affect {
	t_variable variable;
	t_expr valeur;
}

typedef struct t_class {
	char* name;
	t_method constructor
	t_method* methods;
	t_varIdent* attributes;
	t_class superClass;
	
}

typedef struct t_method {
	char* name;
	t_type returnType;
	t_varIdent* parametres;
	t_instr* instructions;
	bool isRedef;
	
}

typedef union t_expr {
    t_variable ident;
    t_value constante;
    struct {
        char* fieldName;
        t_expr expression;
    } t_selection;
    t_instanciation instanciation;
};

typedef union t_instr {
	t_instr* bloc;
	t_return _return;
    struct {
        t_expr exprG;
        t_expr exprD;
    } t_affect;
	struct {
	    t_expr condition;
	    t_instr instrThen;
	    t_instr instrElse;
    } t_ite;
}

typedef struct t_variable {
	t_varIdent ident;
	t_value value;
	
}

typedef struct t_varIdent {
	char* name;
	t_class* _type;
}


typedef struct t_value {
	t_class* _type;
}



typedef struct t_cast {
    t_expr expression;
    t_class newType;
}
