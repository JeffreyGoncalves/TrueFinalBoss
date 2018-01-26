#include <stdio.h>
#include "tp.h"
#include "tp_y.h"
#include <fcntl.h>

void makeCode();


extern char* strdup(const char *);

/*int main(int argc, char **argv) {

	printf("Construction de l'arbre de test\n");

    TreeP tree = makeNode(2, SUM);
    setChild(tree, 0, makeLeafInt(CST, 2));
    setChild(tree, 1, makeNode(2, MULT));
    setChild(getChild(tree, 1), 0, makeLeafInt(CST, 3));
    setChild(getChild(tree, 1), 1, makeLeafInt(CST, 4));

	FILE* pFile;
	pFile = fopen ("myfile.txt","w");

	printf("Ecriture du code\n");
	makeCode(tree, pFile);

    fclose (pFile);
	return 0;
}*/

void makeCode(TreeP tree, FILE* pFile) {

	if (pFile==NULL) {
        printf("erreur fichier\n");
        return;
	}

	switch(tree->op) {
		case SUM :
        fputs (" -- Il y a une somme\n", pFile);
        makeCode(tree->u.children[0], pFile);
        makeCode(tree->u.children[1], pFile);
        fputs ("ADD\n", pFile);
        fputs (" -- Fin de la somme\n", pFile);
		break;
		case MULT :
        fputs (" -- Il y a un produit\n", pFile);
        makeCode(tree->u.children[0], pFile);
        makeCode(tree->u.children[1], pFile);
        fputs ("MULT\n", pFile);
        fputs (" -- Fin du produit\n", pFile);
		break;
		case CST :
        fprintf ( pFile, "PUSHI %d\n", tree->u.val);
		break;
		default :
        fputs (" -- Il y a quelque chose\n", pFile);
		break;

	}

}