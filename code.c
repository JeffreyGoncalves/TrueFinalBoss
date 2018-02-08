#include "code.h"

extern char* strdup(const char *);

int nbObjets;
t_object* obj;
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
}
*/
void makeCodeClasse(t_class* class, FILE* pFile) {
    ;
}
void makeCodeObjet(t_object* obj, FILE* pFile) {
    /*int offsetSuivant = 0; // Ne sera peut-etre pas utile */
    while (obj != NULL) {
        int nbChamps = 0;
        nbChamps = tailleAlloc(obj->attributes);
        t_method* methode = obj->methods;
        while(methode != NULL) {
            nbChamps++;
            methode = methode->next;
        }
        fprintf(pFile,"ALLOC %d\n", nbChamps);
        /*obj->offset = offsetSuivant;
        offsetSuivant++;*/
        obj = obj->next;
    }
}

int tailleAlloc(VarDeclP decl) {
    int taille = 0;
    while(decl != NULL) {
        taille++;
        decl = decl->next;
    }
    return taille;
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
            makeCode(getChild(tree, 0), pFile);/* On ecrit le code donnant l'offset de l'expression */
            fprintf(pFile, "LOAD %d", getOffsetAttr(obj->attributes, getChild(tree, 1)->u.str));
        break;
        case _ID :
            /* on empile l'offset de l'instance de classe ou de l'objet independant ayant cet identifiant */
            fprintf(pFile, "PUSHG %d", getOffsetObj(obj, tree->u.str));
        break;
        case CAST :
            fprintf(pFile, "-- Il y a un cast\n" );
            /*je vois pas comment faire celui-là*/
        break;
        case E_CALL_METHOD :
            fprintf(pFile, "Il y a un appel de methode\n");
            /*faudra probablement faire une fonction a part*/
        break;
        case I_ITE :
            fprintf(pFile, "Il y a un bloc If Then Else\n");
            /*ça c'est comme les tds/dm/exams ez*/
        break;
        case I_BLOC :
            fprintf(pFile, "Il y a un bloc d'instructions\n");
            /*plusieurs instructions a lire*/
        break;
        case I_RETURN :
        /*pas sûr sûr du truc*/
        break;
        case I_AFF :
            fprintf(pFile, "Il y a une affectation\n");
            /*dupliquer la valeur dans la pile uniquement ?*/
        break;
        case CLASS_NAME :
            /*dunno how to do dis Oo*/
        break;
        case _OVERRIDE :
            /*pas sûr sûr*/
        break;
		default :
            fprintf (pFile, "-- Il y a quelque chose\n");
		break;
	}

}

int getOffsetObj(t_object* obj, char* nom) {
    int i = 0;
    while (obj != NULL && strcmp(obj->name, nom)) {
        i++;
        obj = obj->next;
    }
    if (obj == NULL)
        printf("Attribut introuvable");
    return i;
}

/* utilisable pour les attributs dans un objet ou dans une classe */
int getOffsetAttr(VarDeclP decl, char* nom) {
    int i = 0;
    while (decl != NULL && strcmp(decl->name, nom)) {
        i++;
        decl = decl->next;
    }
    if (decl == NULL)
        printf("Attribut introuvable");
    return i;
}

void InitTV(list_ClassObjP env, FILE* pFile){

	char* label = malloc(3*sizeof(char));
	label[0] = 'm';
	int i = 0;
	fprintf(pFile, "-- Initilisation des tables virtuelles\n");
	fprintf(pFile, "init :");
	while(env->listClass->next != NIL(t_class)){
		fprintf(pFile, "\tALLOC %d\n",tailleAlloc(env->listClass->parametres));
		fprintf(pFile, "\t\tDUPN %d\n",1);
		while(env->listClass->methods->next != NIL(t_method)){
			sprintf(label+1,"%d",i+1);
			fprintf(pFile, "\t\tPUSHA %s\n",label);
			fprintf(pFile, "\t\tSTORE %d\n",i);
			if(env->listClass->methods->next != NIL(t_method)){
				fprintf(pFile, "\t\tDUPN %d\n",1);
			}
			i++;
			env->listClass->methods = env->listClass->methods->next;
		}
		i=0;
		env->listClass = env->listClass->next;
	}
	fprintf(pFile, "-- Debut main\n");
	fprintf(pFile, "\t\tJUMP %s\n","main");
}
