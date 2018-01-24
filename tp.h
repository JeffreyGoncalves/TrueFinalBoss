#include <stdlib.h>

#define TRUE 1
#define FALSE 0

typedef unsigned char bool;

/* deux macros pour eviter de faire des malloc en se trompant de type : on
 * s'oblige a expliciter le type d'objet a allouer et on récupere une
 * de type pointeur sur le type alloue.
 * Exemple: Allouer 3 objets de la structure Tree (voir ci-dessous)
 * TreeP p := NEW(3, Tree);
 *
 * De meme, sobloiger à mettre un type au NIL ci-dessous permet souvent
 * d'eviter de confondre entre des pointeurs de type differents.
 */
#define NEW(howmany, type) (type *) calloc((unsigned) howmany, sizeof(type))
#define NIL(type) (type *) 0

/* Etiquettes additionnelles pour les arbres de syntaxe abstraite.
 * Certains tokens servent directement d'etiquette. Attention ici a ne pas
 * donner des valeurs identiques a celles des tokens.
 *
 * Il y a surement des choses a recuperer en plus de ce que vous avez
 * produit pour le tp.
 */
#define SUM  1
#define MIN  2
#define MULT 3
#define DIVI 4
#define NE	5
#define EQ	6
#define LT	7
#define LE	8
#define GT	9
#define GE	10
#define AND 11


/* Codes d'erreurs. Cette liste n'est pas obligatoire ni limitative */
#define NO_ERROR	0
#define USAGE_ERROR	1
#define LEXICAL_ERROR	2
#define SYNTAX_ERROR    3
#define CONTEXT_ERROR	40	/* default value for this stage */
#define DECL_ERROR	41	/* scope problem */
#define TYPE_ERROR	42	/* type checking problem */
#define OTHER_CONTEXTUAL_ERROR	43 /* replace by something more precise */
#define EVAL_ERROR	50	/* dubious when writing a compiler ! */
#define UNEXPECTED	10O


/* Adapt as needed. Currently it is simply a list of names ! */
typedef struct _varDecl {
  char *name;
  struct _varDecl *next;
} VarDecl, *VarDeclP;


/* la structure d'un arbre (noeud ou feuille) */
typedef struct _Tree {
  short op;         /* etiquette de l'operateur courant */
  short nbChildren; /* nombre de sous-arbres */
  union {
    char *str;      /* valeur de la feuille si op = Id ou STR */
    int val;        /* valeur de la feuille si op = Cste */
    VarDeclP lvar;  /* ne pas utiliser tant qu'on n'en a pas besoin :-) */
    struct _Tree **children; /* tableau des sous-arbres */
  } u;
} Tree, *TreeP;

/* PERSO */

typedef struct t_value{
	enum{
		Int, String, Class, Void, Object
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
	struct t_expr* variable;
	struct t_expr* valeur;
}t_affect;

typedef struct t_expr {
	enum{
		Affect, Add, Soustr, Mult, Div, Equal, Ineq, And, Leaf, Cast, New /*No operator*/
	}label_op;
	
	union{
		struct t_expr** fils;
		t_variable* ident;
		t_value* constante;
		struct {
			struct t_variable* fieldName;
			struct t_expr* expression;
		}selection;
		struct t_instanciation* instanciation;
		struct t_cast* cast;
		struct{
			struct t_expr* expression;	
			struct t_variable* name;
			struct t_listParam* list;
		}callMethod;
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
		Bloc, Return, Affectation, IfThenElse, Expression
	}Instr_enum;
	union {
		struct t_instr* bloc;
		t_expr* expr;
		t_affect* aff;
		struct {
			t_expr* condition;
			struct t_instr* instrThen;
			struct t_instr* instrElse;
		}ite;
		struct t_champ* champ;
	}instr;
}t_instr;

typedef struct t_varIdent{
	char* name;
	t_class* _type;
}t_varIdent;

typedef struct t_listParam{
    t_varIdent* varIdent;
    struct t_listParam* listParam;
}t_listParam;

typedef struct t_cast{
    t_expr* expression;
    t_class* newType;
}t_cast;

typedef struct t_init{
	t_expr* expression_to_affect;
}t_init;

typedef struct t_champ{
	t_variable* ident1;
	t_variable* ident2;
}t_champ;

/* FIN PERSO */

typedef union
      { char *S;
        char C;
	int I;
	TreeP pT;
	VarDeclP pV;
	t_value* pvalue;
	t_object* pobject;
	t_class* pclass;
	t_variable*  pvariable;
	t_affect* paffect;
	t_expr* pexpr;
	t_method* pmethod;
	t_instanciation* pinstanciation;
	t_instr* pinstr;
	t_cast* pcast;
	t_init* pinit;
	t_champ* pchamp;
	t_listParam* plistParam;
} YYSTYPE;

#define YYSTYPE YYSTYPE

/* DECLARATION OF DOOM */
t_instr* makeInstruction(short cas, ...);
t_affect* makeAff(t_expr* var, t_expr* expr);
t_expr* makeExprSelect(t_variable* varId, t_expr* expres);
t_expr* makeExprInst(t_instanciation* inst);
t_expr* makeExprCste(short op, t_value* cste);
t_expr* makeExprVar(t_variable* var);
t_expr* makeExprCast(t_cast* new_cast);
t_expr* makeExpr(short op, ...);
t_init* makeInit(t_expr *expr);
t_champ* makeChamp(t_variable* id1, t_variable* id2);
t_method* makeMethod(char* name, t_class* returnType, short nbParametres, t_varIdent** parametres, int* isRedef);
t_cast* makeCast(t_variable* class_id, t_expr* expr_to_cast);
t_instanciation* makeInstanciation(t_variable* class_id, t_expr** args);
t_expr* makeExprCallMethod(t_expr* expr1, t_variable* var, t_listArg* list);
t_listParam* makeListParam(t_varIdent* var, t_listParam* list);
