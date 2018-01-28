#include <stdio.h>
#include "tp.h"
#include <fcntl.h>

void makeCode(TreeP tree, FILE* pFile);

extern char* strdup(const char *);

t_variable** varGlobales;
int nbVarGlobales;


/*int main(int argc, char **argv) {

	printf("Construction de l'arbre de test\n");

    TreeP tree = makeNode(2, SUM);
    setChild(tree, 0, makeLeafInt(CST, 2));
    setChild(tree, 1, makeNode(2, MULT));
    setChild(getChild(tree, 1), 0, makeLeafInt(CST, 3));
    setChild(getChild(tree, 1), 1, makeLeafInt(CST, 4));

    varGlobales = NEW(100, t_variable*);
    nbVarGlobales = 0;

	FILE* pFile;
	pFile = fopen ("myfile.txt","w");
	if (pFile==NULL) {
        printf("erreur fichier\n");
        return;
	}
	printf("Ecriture du code\n");

	makeCode(tree, pFile);

    fclose (pFile);
	return 0;
}*/

/*
Une variable globale = forcement un objet ?
"DECL_METH_1DECL_METH_2" dans l'arbre de obj.txt => normal ?
Rajouter des champs a t_variable (offset et portee)
Explications generales sur l'arbre
explications sur varDecl
*/


void makeCode(TreeP tree, FILE* pFile) {

    if(tree == NULL) {
        printf("-- null tree");
        return;
    }

	switch(tree->op) {
        case OBJ :
        fprintf(pFile, "-- Il y a une declaration d'objet\n");
        int nbChamps = 0;
        TreeP t = getChild(tree, 1);
        while(t!=NULL) {
            ++nbChamps;
            t = getChild(t, 1);
        }
        break;
	    case LIST_CLASS :
        makeCode(getChild(tree, 0), pFile);
        fprintf(pFile, "-- Il y a une definition de classe\n");
        break;
	    case LIST_VAR_DEF :
        fprintf(pFile, "-- Il y a une definition de variable\n");
        makeCode(getChild(tree, 1), pFile);
        break;
		case SUM :
        fprintf (pFile, "-- Il y a une somme\n");
        makeCode(getChild(tree, 0), pFile);
        makeCode(getChild(tree, 1), pFile);
        fprintf (pFile, "ADD\n");
        fprintf (pFile, "-- Fin de la somme\n");
		break;
		case MULT :
        fprintf (pFile, "-- Il y a un produit\n");
        makeCode(getChild(tree, 0), pFile);
        makeCode(getChild(tree, 1), pFile);
        fprintf (pFile, "MULT\n");
        fprintf (pFile, "-- Fin du produit\n");
		break;
		case CST :
        fprintf (pFile, "PUSHI %d\n", tree->u.val);
		break;
		default :
        fprintf (pFile, "-- Il y a quelque chose\n");
		break;

	}

}
