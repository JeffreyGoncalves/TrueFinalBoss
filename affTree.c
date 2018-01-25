#include <stdio.h>
#include <stdlib.h>

void affTree(char *space, TreeP tree)
{
	//Caracteres d'affichage de l'arbre
	char arrow[100] = "";
	char bar[100] = "";
	strcat(arrow, space);
	strcat(bar, space);
	strcat(arrow, "->");
	strcat(bar, "|");
	
	//Affichage du champ op de tree
	printf("%s\n%s\n", bar, bar);
	printf("%s %d", arrow, tree->op);
	
	//On regarde s'il y a des enfants
	if(tree->nbChildren > 0)
	{
		int i;
		strcat(space, "\t");
		for(i=0;i<tree->nbChildre;i++)
		{
			affTree(space, tree->u.children[i];
		}
	}
}
