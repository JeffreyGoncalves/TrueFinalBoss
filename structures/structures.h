#include <stdlib.h>

typedef struct t_value{
	enum{
		INT, STRING, CL, VOID, OBJ
	}Type_enum;
	union{
		struct t_class* Class;
		struct t_object* Object;
	}Type;
}t_value;

typedef struct t_class{
	char* name;
	struct t_method** constructor;
	struct t_method** methods;
	struct t_varIdent** attributes;
	struct t_class* superClass;
}t_class;

typedef struct t_object{
	char* name;
	struct t_method* constructor;
	struct t_method** methods;
	struct t_varIdent** attributes;
}t_object;

typedef struct t_variable{
	struct t_varIdent* ident;
	t_value* value;
}t_variable;

typedef struct t_affect{
	t_variable* variable;
	struct t_expr* valeur;
}t_affect;

typedef struct t_expr {
	enum{
		AFFECT, PLUS, SOUSTR, MULT, DIV, EQUAL, INEQ, AND, LEAF /*No operator*/
	}label_op;
	
	union{
		struct t_expr** fils;
		t_variable* ident;
		t_value* constante;
		struct {
			struct t_varIdent* fieldName;
			struct t_expr* expression;
		}selection;
		struct t_instanciation* instanciation;
	}elem;
	
}t_expr;

typedef struct t_method{
	char* name;
	t_class* returnType;
	short nbParametre;
	struct t_varIdent** parametres;
	struct t_instr** instructions;
	int* isRedef;
}t_method;

typedef struct t_instanciation{
	t_class* class;
	short nbArgs;
	t_expr** args;
}t_instanciation;

typedef struct t_instr{
	enum {
		BLOC, RETURN, AFFECTATION, IFTHENELSE
	}Instr_enum;
	union {
		struct t_bloc* bloc;
		t_expr* _return;
		t_affect* aff;
		struct {
			t_expr* condition;
			struct t_instr* instrThen;
			struct t_instr* instrElse;
		}ite;
	}instr;
}t_instr;

typedef struct t_bloc{
	

typedef struct t_varIdent{
	char* name;
	t_class* _type;
}t_varIdent;

typedef struct t_cast{
    t_expr* expression;
    t_class* newType;
}t_cast;

/*typedef struct t_expr{
	enum{
		IDENT, CONSTANTE, SELECTION, INSTANCIATION
	}Expr_enum;
	union{
		t_variable* ident;
		t_value* constante;
		struct {
			struct t_varIdent* fieldName;
			struct t_expr* expression;
		}selection;
		struct t_instanciation* instanciation;
	};
}t_expr;*/
