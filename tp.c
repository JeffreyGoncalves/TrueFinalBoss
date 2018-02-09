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

/* Peut servir a controloer le niveau de 'verbosite'.
 * Par defaut, n'imprime que le resultat et les messages d'erreur
 */
bool verbose = FALSE;

/* Peut servir a controler la generation de code. Par defaut, on produit le code
 * On pourrait avoir un flag similaire pour s'arreter avant les verifications
 * contextuelles (certaines ou toutes...)
 */
bool noCode = FALSE;

/* Pour controler la pose de points d'arret ou pas dans le code produit */
bool debug = FALSE;

/* code d'erreur a retourner */
int errorCode = NO_ERROR; /* defini dans tp.h */

FILE *out; /* fichier de sortie pour le code engendre */

int main(int argc, char **argv) {
  int fi;
  int i, res;

  out = stdout;
  for(i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'd': case 'D':
	debug = TRUE; continue;
      case 'v': case 'V':
	verbose = TRUE; continue;
      case 'e': case 'E':
	noCode = TRUE; continue;
      case '?': case 'h': case 'H':
	fprintf(stderr, "Appel: tp -v -e -d -o file.out programme.txt\n");
	exit(USAGE_ERROR);
      case'o':
	  if ((out= fopen(argv[++i], "w")) == NULL) {
	    fprintf(stderr, "erreur: Cannot open %s\n", argv[i]);
	    exit(USAGE_ERROR);
	  }
	break;
      default:
	fprintf(stderr, "Option inconnue: %c\n", argv[i][1]);
	exit(USAGE_ERROR);
      }
    } else break;
  }

  if (i == argc) {
    fprintf(stderr, "Fichier programme manquant\n");
    exit(USAGE_ERROR);
  }

  if ((fi = open(argv[i++], O_RDONLY)) == -1) {
    fprintf(stderr, "erreur: Cannot open %s\n", argv[i-1]);
    exit(USAGE_ERROR);
  }
  close(0); dup(fi); close(fi);
  
  res = yyparse();

  if (out != NIL(FILE) && out != stdout) fclose(out);
  return res ? SYNTAX_ERROR : errorCode;
}


void setError(int code) {
  errorCode = code;
  if (code != NO_ERROR) {
    noCode = TRUE;

    /* la ligne suivante peut servir a "planter" volontairement le programme
     * des qu'une de vos fonctions detectent une erreur et appelle setError.
     * Si vous executez le rpogramme sous le debuggeur vous aurez donc la main
     * et pourrez examiner la pile d'execution.
     */
    /*  abort(); */
  }
  if(code == NO_EXISTING_METHOD){
    printf("Cette methode n'existe pas (cf ligne %d)\n",yylineno);
  } 

  else if(code == PARAM_ERROR_1){
  	printf("Une methode/Un constructeur ne possede pas le bon nombre de parametres : ligne %d\n",yylineno);
  }

  else if(code == PARAM_ERROR_2){
  	printf("Un parametre ou plusieurs parametres n'ont pas le bon type : ligne %d\n",yylineno);
  }
  
  else if(code == CLASS_NOT_FOUND){
	printf("L'une des classes utilisee a cette ligne n'existe pas : ligne %d\n",yylineno);
  }
}


/* yyerror:  fonction importee par Bison et a fournir explicitement. Elle
 * est appelee quand Bison detecte une erreur syntaxique.
 * Ici on se contente d'un message a minima.
 */
void yyerror(char *ignore) {
  printf("erreur de syntaxe: Ligne %d\n", yylineno);
  setError(SYNTAX_ERROR);
}



/* ****** Fonctions pour la construction d'AST   ********************
 *
 * Ajoutez vos propres constructeurs, si besoin
 *
 */

/* Tronc commun pour la construction d'arbre. Normalement on ne l'appelle
 * pas directement. Elle ne fait qu'allouer, sans remplir les champs
 */
TreeP makeNode(int nbChildren, short op) {
  TreeP tree = NEW(1, Tree);
  tree->op = op;
  tree->nbChildren = nbChildren;
  tree->u.children = nbChildren > 0 ? NEW(nbChildren, TreeP) : NIL(TreeP);
  return(tree);
}
/* Construction d'un arbre a nbChildren branches, passees en parametres.
 * Pour comprendre en detail (si necessaire), regardez un tutorial sur
 * comment on passe un nombre variable d'arguments à une fonction et
 * comment on recupere chacun de ces arguments.
 * cf va_list, va_start, va_arg et va_end.
 * makeTree prend donc toujours au moins 2 arguments
 */
TreeP makeTree(short op, int nbChildren, ...) {
  va_list args;
  TreeP tree = makeNode(nbChildren, op);
  va_start(args, nbChildren);
  int i;
  for(i = 0; i < nbChildren; i++) {
    tree->u.children[i] = va_arg(args, TreeP);
  }
  va_end(args);
  return(tree);
}

/* Retourne le rank-ieme fils d'un arbre (de 0 a n-1) */
TreeP getChild(TreeP tree, int rank) {
  if (tree->nbChildren < rank -1) {
    fprintf(stderr, "Incorrect rank in getChild: %d\n", rank);
    abort(); /* plante le programme en cas de rang incorrect */
  }
  return tree->u.children[rank];
}


void setChild(TreeP tree, int rank, TreeP arg) {
  if (tree->nbChildren < rank -1) {
    fprintf(stderr, "Incorrect rank in getChild: %d\n", rank);
    abort(); /* plante le programme en cas de rang incorrect */
  }
  tree->u.children[rank] = arg;
}

/* Constructeur de feuille qui cree une classe avec juste un nom */
/*TreeP makeLeafClass(short op, char* className) {
	TreeP 
}*/

/* Constructeur de feuille dont la valeur est une chaine de caracteres */
TreeP makeLeafStr(short op, char *str) {
  TreeP tree = makeNode(0, op);
  tree->u.str = str;
  return tree;
}

/* Constructeur de feuille dont la valeur est un entier */
TreeP makeLeafInt(short op, int val) {
  TreeP tree = makeNode(0, op);
  tree->u.val = val;
  return(tree);
}

/* Constructeur de feuille : liste de paramètre ****************************************************/
TreeP makeLeafParam(short op, VarDeclP p) {			
  	TreeP tree = makeNode(0, op);
  	tree->u.lvar = p;
  return(tree);
}

VarDeclP lastList(VarDeclP o){
	while(o->next != NULL){
		o = o->next;	
	}
	return o;
}

VarDeclP makeVarDeclP(char *nom, char *type,TreeP sArbre, int i){
	VarDeclP param = NEW(1, VarDecl);
	param->name = nom;
	param->coeur = NEW(1,t_variable);
	param->coeur->_type = NEW(1, t_class);
	param->coeur->_type->name = type;
	param->coeur->value = sArbre;
	param->coeur->res.i = i;
	return param;
}

void ajouteParam(TreeP list, VarDeclP p){
	VarDeclP dernier = lastList(list->u.lvar);
	dernier->next = p;
}
/**************************************************************************************************/

/* Constructeur de feuille dont la valeur est une declaration */
TreeP makeLeafLVar(short op, VarDeclP lvar) {
  TreeP tree = makeNode(0, op);
  tree->u.lvar = lvar;
  return(tree);
}

/* Constructeur de feuille dont la valeur est une declaration */
TreeP makeLeafLparam(short op, VarDeclP id, VarDeclP lvar) {
  TreeP tree = makeNode(0, op);
  tree->u.lvar = lvar;
  return(tree);
}

/* FONCTIONS PERSO */

/* Constructeur expression non feuille *//*
t_expr* makeExpr(short op, ...){
	va_list args;
	t_expr* expr = NEW(1, t_expr);
	expr->label_op = op;

	va_start(args, op);
	int i;
	for(i = 0; i < 2; i++) {
    		expr->elem.fils[i] = va_arg(args, t_expr*);
  	}
	va_end(args);
	return(expr);
}*/

/*********	Affichage de l'arbre *********/

void printOP(short op)
{
	switch(op)
	{
		case SUM : 
			printf("SUM");
			break;
		case MIN : 
			printf("MIN");
			break;
		case MULT : 
			printf("MULT");
			break;
		case DIVI : 
			printf("DIVI");
			break;
		case NE : 
			printf("NE");
			break;
		case EQ : 
			printf("EQ");
			break;
		case LT : 
			printf("LT");
			break;
		case LE : 
			printf("LE");
			break;
		case GT : 
			printf("GT");
			break;
		case GE : 
			printf("GE");
			break;
		case AND : 
			printf("AND");
			break;
		case CST : 
			printf("CST");
			break;
		case _STR : 
			printf("_STR");
			break;
		case _ID : 
			printf("_ID");
			break;
		case CAST : 
			printf("CAST");
			break;
		case DECL : 
			printf("DECL");
			break;
		case INST : 
			printf("INST");
			break;
		case E_CALL_METHOD : 
			printf("E_CALL_METHOD");
			break;
		case E_SELECT : 
			printf("E_SELECT");
			break;
		case I_ITE : 
			printf("I_ITE");
			break;
		case I_BLOC : 			
			printf("I_BLOC");
			break;
		case I_RETURN : 
			printf("I_RETURN");
			break;
		case I_AFF : 
			printf("I_AFF");
			break;
		case I_EXPRRELOP : 
			printf("I_EXPRRELOP");
			break;
		case LIST_ARG : 
			printf("LIST_ARG");
			break;
		case LIST_INST :
			printf("LIST_INST");
			break;
		case LIST_PARAM : 
			printf("LIST_PARAM");
			break;
		case _EXTENDS : 
			printf("_EXTENDS");
			break;
		case _VAR : 
			printf("_VAR");
			break;
		case LIST_CHAMP : 
			printf("LIST_CHAMP");
			break;
		case CLAS : 
			printf("CLAS");
			break;
		case OBJ : 
			printf("OBJ");
			break;
		case _CLASS : 
			printf("_CLASS");
			break;
		case CLASS_NAME : 
			printf("CLASS_NAME");
			break;
		case _OVERRIDE : 
			printf("_OVERRIDE");
			break;
		case DECLA_OBJECT: 
			printf("DECLA_OBJECT");
			break;
		case LIST_VAR_DEF : 
			printf("LIST_VAR_DEF");
			break;
		case VAR_DEF_METH : 
			printf("VAR_DEF_METH");
			break;
		case VAR_DEF_CHAMP : 
			printf("VAR_DEF_CHAMP");
			break;
		case DECL_METH_1 : 
			printf("DECL_METH_1");
			break;
		case DECL_METH_2 : 
			printf("DECL_METH_2");
			break;
		case LIST_CLASS: 
			printf("LIST_CLASS");
			break;
		case PROG: 
			printf("PROG");
			break;
		default :
			printf("unknown");
			break;
	}
	printf("\n");
}

void barAff(int stage)
{
	int i;
	for(i=0;i<stage;i++)
		printf("|   ");
	printf("\n");
}

void arrowAff(int stage)
{
	int i;
	for(i=0;i<stage;i++)
	{
		if(i != stage-1) printf("|   ");
		else printf("---");
	}
	printf("-> ");
}

void affTree(TreeP tree, int stage)
{
	int i;
	
	/*Affichage du champ op de tree*/
	barAff(stage);
	barAff(stage);
	arrowAff(stage);
	if(tree != NULL)
		printOP(tree->op);
	else printf("null\n");
	
	/*On regarde s'il y a des enfants*/
	if(tree != NULL && tree->nbChildren > 0)
	{
		for(i=0;i<tree->nbChildren;i++)
		{
			affTree(tree->u.children[i], stage+1);
		}
	}
}
