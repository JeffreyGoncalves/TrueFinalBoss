#include <stdlib.h>
#include <string.h>

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
#define CST 12
#define _STR 13
#define _ID 14
#define CAST 15
#define DECL 16
#define INST 17

#define E_CALL_METHOD 18
#define E_SELECT 19

#define I_ITE 20
#define I_BLOC 21
#define I_RETURN 22
#define I_AFF 23
#define I_EXPRRELOP 24
#define I_CHAMP 25

#define LIST_ARG 26
#define LIST_PARAM 27
#define _EXTENDS 28
#define _VAR 29
#define LIST_CHAMP 30

#define CLAS 31
#define OBJ 32
#define _CLASS 33
#define CLASS_NAME 34
#define _OVERRIDE 35
#define DECLA_OBJECT 36
#define LIST_VAR_DEF 37
#define VAR_DEF_METH 38
#define VAR_DEF_CHAMP 39
#define DECL_METH_1 44
#define DECL_METH_2 51
#define LIST_CLASS 45
#define LIST_INST 53
#define EXPR_RELOP 52

#define PROG 100

/* Codes d'erreurs. Cette liste n'est pas obligatoire ni limitative */
#define NO_ERROR	0
#define USAGE_ERROR	1
#define LEXICAL_ERROR	2
#define SYNTAX_ERROR    3
#define CONTEXT_ERROR	40	/* default value for this stage */
#define DECL_ERROR	41	/* scope problem */
#define TYPE_ERROR	42	/* type checking problem */
#define OTHER_CONTEXTUAL_ERROR	43 /* replace by something more precise */
#define REDECL_ERROR 44 /* Redeclaration problem */
#define RESERV_DECL_ERROR 45 /* Forbidden use of a reserved ID (this, super, result) */
#define RETURN_ERROR 46 /* Forbidden use of return */
#define NO_EXISTING_METHOD 47 /*This method does not exist in its class*/
#define PARAM_ERROR_1 48 /*Wrong number of parameters*/
#define PARAM_ERROR_2 49 /*A parameter or several parameters do not have the right type*/
#define EVAL_ERROR	50	/* dubious when writing a compiler ! */
#define OVERRIDE_ERROR 51 /* Override forbidden for methods of object */
#define VAR_NOT_FOUND 52 /* Variable not found */
#define CLASS_NOT_FOUND 404 /* Class is not found :) */
#define UNEXPECTED	10O


/* Adapt as needed. Currently it is simply a list of names ! */
typedef struct _varDecl {
	char *name;
	struct t_variable* coeur;
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
	struct t_method* constructor;
	VarDeclP parametres;
	struct t_method* methods;
	VarDeclP attributes;
	struct t_class* superClass;
	struct t_class* next;
}t_class;

typedef struct t_object{
	char* name;
	struct t_method* methods;
	VarDeclP attributes;
	struct t_object* next;
}t_object;

typedef struct t_variable{
	struct t_class* _type;
	struct t_object* _obj;
	TreeP value;
	union{
		int i;
		char* s;
	}res;
}t_variable;

typedef struct list_ClassObj{
	t_class* listClass;
	t_object* listObj;
}list_ClassObj, *list_ClassObjP;

typedef struct t_method{
	char* name;
	t_class* returnType;
	short nbParametres ;
	VarDeclP parametres;
    TreeP bloc;
	int isRedef;
	struct t_method* next;
}t_method;

/* FIN PERSO */

typedef union
{ 	
	char *S;
    char C;
	int I;
	TreeP pT;
	VarDeclP pV;
	t_value* pvalue;
	t_object* pobject;
	t_class* pclass;
	t_variable*  pvariable;
	t_method* pmethod;
} YYSTYPE;

#define YYSTYPE YYSTYPE

/* DECLARATION OF DOOM */
TreeP makeNode(int nbChildren, short op);
TreeP makeTree(short op, int nbChildren, ...);
void setChild(TreeP tree, int rank, TreeP arg);
t_method* makeMethod(char* name, t_class* returnType, short nbParametres, VarDecl** parametres, int* isRedef);
TreeP makeLeafInt(short op, int val);
TreeP makeLeafLVar(short op, VarDeclP lvar);
TreeP makeLeafStr(short op, char *str);
VarDeclP lastList(VarDeclP o);
TreeP makeLeafParam(short op, VarDeclP p);
VarDeclP makeVarDeclP(char *nom, char *type,TreeP sArbre);
void ajouteParam(TreeP list, VarDeclP p);
TreeP getChild(TreeP tree, int rank);
void affTree(TreeP tree, int stage);
