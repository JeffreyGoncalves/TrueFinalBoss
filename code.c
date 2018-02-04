#include "code.h"

extern char* strdup(const char *);

int nbObjets;

/*
int main(int argc, char **argv) {

	printf("Construction de l'arbre de test\n");

    TreeP tree = makeNode(2, SUM);
    setChild(tree, 0, makeLeafInt(CST, 2));
    setChild(tree, 1, makeNode(2, MULT));
    setChild(getChild(tree, 1), 0, makeLeafInt(CST, 3));
    setChild(getChild(tree, 1), 1, makeLeafInt(CST, 4));

    t_object* obj = NEW(1, t_object);
    VarDeclP declA = NEW (1, VarDecl);
    declA = makeVarDeclP("A", "Integer", NULL);
    obj->attributes = declA;

	FILE* pFile;
	pFile = fopen ("myfile.txt","w");
	if (pFile==NULL) {
        printf("erreur fichier\n");
        return 0;
	}
	printf("Ecriture du code\n");

    fprintf(pFile, "START\n");
	makeCode(tree, pFile);
    fprintf(pFile, "PUSHS \"resultat = \"\n");
    fprintf(pFile, "WRITES\n");
    fprintf(pFile, "WRITEI\n");
    fprintf(pFile, "PUSHS\"\\n\"\n");
    fprintf(pFile, "WRITES\n");
    fprintf(pFile, "STOP\n");

    fclose (pFile);
	return 0;
}*/

void makeCodeClasse(t_class* class, FILE* pFile) {
    ;
}
void makeCodeObjet(t_object* obj, FILE* pFile) {
    int offsetSuivant = 0;
    while (obj != NULL) {
        int nbChamps = 0;
        nbChamps = tailleAlloc(obj->attributes);
        t_method* methode = obj->methods;
        while(methode != NULL) {
            nbChamps++;
            methode = methode->next;
        }
        fprintf(pFile,"ALLOC %d\n", nbChamps);
        obj->offset = offsetSuivant;
        offsetSuivant += nbChamps;
        obj = obj->next;
    }
}

int tailleAlloc(VarDeclP decl) {
    decl->next;
    int taille = 0;
    while(decl != NULL) {
        taille++;
        decl = decl->next;
    }
}

void makeCode(TreeP tree, FILE* pFile) {

    if(tree == NULL) {
        printf("-- null tree");
        return;
    }

	switch(tree->op) {

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
            fprintf (pFile, "MUL\n");
            fprintf (pFile, "-- Fin du produit\n");
		break;
		case MIN :
            fprintf (pFile, "-- Il y a une soustraction\n");
		    if (tree->nbChildren == 2) {
                makeCode(getChild(tree, 0), pFile);
                makeCode(getChild(tree, 1), pFile);
		    } else {
                fprintf (pFile, "PUSHI 0\n");
                makeCode(getChild(tree, 0), pFile);
		    }
            fprintf (pFile, "SUB\n");
            fprintf (pFile, "-- Fin de la soustraction\n");
		break;
		case DIVI :
            fprintf (pFile, "-- Il y a une division\n");
            makeCode(getChild(tree, 0), pFile);
            makeCode(getChild(tree, 1), pFile);
            fprintf (pFile, "DIV\n");
            fprintf (pFile, "-- Fin du division\n");
		break;
		case EQ :
            fprintf (pFile, "-- Il y a une egalite\n");
            makeCode(getChild(tree, 0), pFile);
            makeCode(getChild(tree, 1), pFile);
            fprintf (pFile, "EQUAL\n");
            fprintf (pFile, "-- Fin de l egalite\n");
		break;
		case NE :
            fprintf (pFile, "-- Il y a une inegalite\n");
            makeCode(getChild(tree, 0), pFile);
            makeCode(getChild(tree, 1), pFile);
            fprintf (pFile, "EQUAL\n");
            fprintf (pFile, "NOT\n");
            fprintf (pFile, "-- Fin de l inegalite\n");
		break;
		case LT :
            fprintf (pFile, "-- Il y a un inferieur strict\n");
            makeCode(getChild(tree, 0), pFile);
            makeCode(getChild(tree, 1), pFile);
            fprintf (pFile, "INF\n");
            fprintf (pFile, "-- Fin du inferieur strict\n");
		break;
		case LE :
            fprintf (pFile, "-- Il y a un inferieur ou egal\n");
            makeCode(getChild(tree, 0), pFile);
            makeCode(getChild(tree, 1), pFile);
            fprintf (pFile, "INFEQ\n");
            fprintf (pFile, "-- Fin du inferieur ou egal\n");
		break;
		case GT :
            fprintf (pFile, "-- Il y a un superieur strict\n");
            makeCode(getChild(tree, 0), pFile);
            makeCode(getChild(tree, 1), pFile);
            fprintf (pFile, "SUP\n");
            fprintf (pFile, "-- Fin du superieur strict\n");
		break;
		case GE :
            fprintf (pFile, "-- Il y a un superieur ou egal\n");
            makeCode(getChild(tree, 0), pFile);
            makeCode(getChild(tree, 1), pFile);
            fprintf (pFile, "SUPEQ\n");
            fprintf (pFile, "-- Fin du superieur ou egal\n");
		break;
		case AND : /* on utilise '&' pour les concatenations */
            fprintf (pFile, "-- Il y a une concatenation\n");
            makeCode(getChild(tree, 0), pFile);
            makeCode(getChild(tree, 1), pFile);
            fprintf (pFile, "CONCAT\n");
            fprintf (pFile, "-- Fin de la concatenation\n");
		break;
		case CST :
            fprintf (pFile, "PUSHI %d\n", tree->u.val);
		break;
		case E_SELECT :

        break;
		default :
            fprintf (pFile, "-- Il y a quelque chose\n");
		break;
	}

}
