#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "tp.h"
#include "tp_y.h"
#include "structures.h"

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

  /* redirige l'entree standard sur le fichier... */
  close(0); dup(fi); close(fi);

  res = yyparse();
  /* si yyparse renvoie 0, le programme en entree etait syntaxiquement correct.
   * Le plus simple est que les verifications contextuelles et la generation
   * de copde soient lancees par les actions associees a la regle de grammaire
   * pour l'axiome. Dans ce cas, quand yyparse renvoie sa valeur on n'a plus
   * rien a faire, sauf fermer les fichiers qui doivent l'etre.
   * Si yyparse renvoie autre chose que 0 c'est que le programme avait une
   * erreur lexicale ou syntaxique
   */
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
  for(int i = 0; i < nbChildren; i++) {
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

/* Constructeur de feuille dont la valeur est une declaration */
TreeP makeLeafLVar(short op, VarDeclP lvar) {
  TreeP tree = makeNode(0, op);
  tree->u.lvar = lvar;
  return(tree);
}

/* FONCTIONS PERSO */

/* Constructeur expression non feuille */
t_expr* makeExpr(short op, ...){
	va_list args;
	t_expr* expr = NEW(1, t_expr);
	expr->label_op = op;

	va_start(args, op);
	for(int i = 0; i < 2; i++) {
    		expr->elem.fils[i] = va_arg(args, t_expr*);
  	}
	va_end(args);

	return(expr);
}

/* Constructeur expression feuille avec variable*/
t_expr* makeExprVar(t_variable* var){
	t_expr* expr = NEW(1, t_expr);
	expr->label_op = 8;
	expr->elem.ident = var;
	return(expr);
}

/* Constructeur expression feuille avec constante*/
t_expr* makeExprCste(t_value* cste){
	t_expr* expr = NEW(1, t_expr);
	expr->label_op = 8;
	expr->elem.constante = cste;
	return(expr);
}

/* Constructeur expression feuille avec instantation*/
t_expr* makeExprInst(t_instanciation* inst){
	t_expr* expr = NEW(1, t_expr);
	expr->label_op = 8;
	expr->elem.instanciation = inst;
	return(expr);
}

/* Constructeur expression feuille avec selection*/
t_expr* makeExprSelect(t_varIdent* varId, t_expr* expres){
	t_expr* expr = NEW(1, t_expr);
	expr->label_op = 8;
	expr->elem.selection.fieldName = varId;
	expr->elem.selection.expression = expres;
	return(expr);
}

/* Constructeur affectation*/
t_affect* makeAff(t_variable* var, t_expr* expr){
	t_affect* aff = NEW(1, t_affect);
	aff->variable = var;
	aff->valeur = expr;
	return(aff);
}

/* Constructeur instruction*/
t_instr* makeInstruction(short cas, ...){
	va_list args;
	t_instr* instr = NEW(1, t_instr);
	instr->Instr_enum = cas;
	
	switch(cas){
		case 0: /*BLOC*/
			/*va_start(args, 2);
			for (int i = 0; i < 2; i++) {
		    		instr->elem.fils[i] = va_arg(args, t_expr);
		  	}
			va_end(args);*/

		case 1: /*RETURN*/
			va_start(args, cas);
			instr->instr._return = va_arg(args, t_expr*);
			va_end(args);

		case 2: /*AFFECTATION*/
			va_start(args, cas);
			instr->instr.aff = va_arg(args, t_affect*);
			va_end(args);

		case 3:	/*IFTHENELSE*/
			va_start(args, cas);
			instr->instr.ite.condition = va_arg(args, t_expr*);
			instr->instr.ite.instrThen = va_arg(args, t_instr*);
			instr->instr.ite.instrElse = va_arg(args, t_instr*);
			va_end(args);
	}

	return(instr);
}
