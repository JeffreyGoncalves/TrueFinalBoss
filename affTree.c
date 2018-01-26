#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _Tree {
  char* op;         /* etiquette de l'operateur courant */
  short nbChildren; /* nombre de sous-arbres */
  union {
    char *str;      /* valeur de la feuille si op = Id ou STR */
    int val;        /* valeur de la feuille si op = Cste */
    struct _Tree **children; /* tableau des sous-arbres */
  } u;
} Tree, *TreeP;

void barAff(int stage)
{
	int i;
	for(i=0;i<stage;i++)
		printf("|\t");
	printf("\n");
}

void arrowAff(int stage)
{
	int i;
	for(i=0;i<stage;i++)
	{
		if(i != stage-1) printf("|\t");
		else printf("--------");
	}
	printf("-> ");
}

void affTree(TreeP tree, int stage)
{
	int i;
	
	//Affichage du champ op de tree
	barAff(stage);
	barAff(stage);
	arrowAff(stage);
	printf("%s\n", tree->op);
	
	//On regarde s'il y a des enfants
	if(tree->nbChildren > 0)
	{
		for(i=0;i<tree->nbChildren;i++)
		{
			affTree(tree->u.children[i], stage+1);
		}
	}
}

int main(int argc, char**argv)
{
	TreeP *tab = malloc(100*sizeof(TreeP));
	int i;
	for(i=0;i<10;i++)
	{
		tab[i] = malloc(sizeof(Tree));
		tab[i]->op = "test_phrase_dont_keep_up_yes";
	}
	tab[0]->nbChildren = 1;
	tab[0]->u.children = malloc(sizeof(TreeP));
	tab[0]->u.children[0] = tab[1];
	
	tab[1]->nbChildren = 2;
	tab[1]->u.children = malloc(2*sizeof(TreeP));
	tab[1]->u.children[0] = tab[2];
	tab[1]->u.children[1] = tab[3];
	
	tab[2]->nbChildren = 2;
	tab[2]->u.children = malloc(2*sizeof(TreeP));
	tab[2]->u.children[0] = tab[4];
	tab[2]->u.children[1] = tab[5];
	
	tab[3]->nbChildren = 2;
	tab[3]->u.children = malloc(2*sizeof(TreeP));
	tab[3]->u.children[0] = tab[6];
	tab[3]->u.children[1] = tab[7];
	
	tab[4]->nbChildren = 2;
	tab[4]->u.children = malloc(2*sizeof(TreeP));
	tab[4]->u.children[0] = tab[8];
	tab[4]->u.children[1] = tab[9];
	
	tab[5]->nbChildren = 0;
	tab[6]->nbChildren = 0;
	tab[7]->nbChildren = 0;
	tab[8]->nbChildren = 0;
	tab[9]->nbChildren = 0;

	affTree(tab[0], 0);
	
	for(i=0;i<10;i++)
	{
		if(i<5)free(tab[i]->u.children);
		free(tab[i]);
	}
	free(tab);
	
	return 0;
}
