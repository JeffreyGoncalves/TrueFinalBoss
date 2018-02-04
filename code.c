#include "code.h"

extern char* strdup(const char *);

t_variable** varGlobales;
int nbVarGlobales;
t_object** objets;
int nbObjets;


int main(int argc, char **argv) {

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
}
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
            TreeP trObj = getChild(tree, 1);
            while(trObj != NULL) {
                if (trObj->op == OBJ) {
                    int temp = 0;
                    nbChamps += tailleAlloc(trObj->u.lvar, &temp);
                }
                trObj = getChild(trObj, 1);
            }
            /*******************************************************************
             * NON !!!!!!!!!! objets est de type t_object**, tu peux pas mettre
             * de TreeP dans un t_object*, ca n'a aucun sens !!!!!!!!!!!!!!!!!!
             * S'IL TE PLAIT RECTIFIE CA !!!
			*******************************************************************/
            objets[nbObjets] = tree;
            /******************************************************************/
            nbObjets++;
            fprintf(pFile, "ALLOC %d\n", nbChamps);
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
		    ; /* En C il ne peut pas y avoir une declaration juste apres un label*/
            TreeP expr =  getChild(tree, 0);
            /* trouver l'objet en memoire (ou en creer un temporaire si c'est une expression) */
            /*t_object* obj;
            if (expr->u.label_op == Leaf) {
                for(int i=0; i<nbObjGlobaux; ++i) {
                    if (strcmp(expr->ident->))
                }
            }
            treeP champ = getChild(tree, 1);*/
            /* trouver le champ de l'objet */
            /*int idChamp = 0;
            while(champ != null) {
                if(strcmp(champ->u.lvar->str, ))
            }*/
        break;
		default :
            fprintf (pFile, "-- Il y a quelque chose\n");
		break;
	}

}

int getOffset(TreeP attribut, int* offset) {
    return 0;
}

int tailleAlloc(VarDeclP varDecl, int* taille) {

    VarDeclP next;

    while(varDecl->next != NULL) {
        varDecl = varDecl->next;
        if (strcmp(varDecl->coeur->_type->name, "Integer") != 0) {
            *taille += 1;
        } else {
            tailleAlloc(varDecl->coeur->_type->attributes, taille);
        }
    }

    return *taille;

}
