#include "code.h"

extern char* strdup(const char *);

int nbObjets;
t_object* obj;


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

void makeCode(TreeP tree,list_ClassObjP env, FILE* pFile) {

	int cpt =0;
    if(tree == NULL) {
        printf("-- null tree");
        return;
    }

    t_method* list = NIL(t_method);
    FILE* file = fopen("../Interprete/ResultGC","r+");
    if(cpt != 1)
    {
    	list = InitMethod(env,file);
    	CallMethod(env,file,list);
    	InitTV(env,file,list);
    	cpt = 1;
    } 	

	switch(tree->op) {

	    case LIST_CLASS :
            makeCode(getChild(tree, 0),env, pFile);
            fprintf(pFile, "-- Il y a une definition de classe\n");
        break;
	    case LIST_VAR_DEF :
            fprintf(pFile, "-- Il y a une definition de variable\n");
            makeCode(getChild(tree, 1),env, pFile);
        break;
		case SUM :
            fprintf (pFile, "-- Il y a une somme\n");
            makeCode(getChild(tree, 0),env, pFile);
            makeCode(getChild(tree, 1),env, pFile);
            fprintf (pFile, "ADD\n");
            fprintf (pFile, "-- Fin de la somme\n");
		break;
		case MULT :
            fprintf (pFile, "-- Il y a un produit\n");
            makeCode(getChild(tree, 0),env, pFile);
            makeCode(getChild(tree, 1),env, pFile);
            fprintf (pFile, "MUL\n");
            fprintf (pFile, "-- Fin du produit\n");
		break;
		case MIN :
            fprintf (pFile, "-- Il y a une soustraction\n");
		    if (tree->nbChildren == 2) {
                makeCode(getChild(tree, 0),env, pFile);
                makeCode(getChild(tree, 1),env, pFile);
		    } else {
                fprintf (pFile, "PUSHI 0\n");
                makeCode(getChild(tree, 0),env, pFile);
		    }
            fprintf (pFile, "SUB\n");
            fprintf (pFile, "-- Fin de la soustraction\n");
		break;
		case DIVI :
            fprintf (pFile, "-- Il y a une division\n");
            makeCode(getChild(tree, 0),env, pFile);
            makeCode(getChild(tree, 1),env, pFile);
            fprintf (pFile, "DIV\n");
            fprintf (pFile, "-- Fin du division\n");
		break;
		case EQ :
            fprintf (pFile, "-- Il y a une egalite\n");
            makeCode(getChild(tree, 0),env, pFile);
            makeCode(getChild(tree, 1),env, pFile);
            fprintf (pFile, "EQUAL\n");
            fprintf (pFile, "-- Fin de l egalite\n");
		break;
		case NE :
            fprintf (pFile, "-- Il y a une inegalite\n");
            makeCode(getChild(tree, 0),env, pFile);
            makeCode(getChild(tree, 1),env, pFile);
            fprintf (pFile, "EQUAL\n");
            fprintf (pFile, "NOT\n");
            fprintf (pFile, "-- Fin de l inegalite\n");
		break;
		case LT :
            fprintf (pFile, "-- Il y a un inferieur strict\n");
            makeCode(getChild(tree, 0),env, pFile);
            makeCode(getChild(tree, 1),env, pFile);
            fprintf (pFile, "INF\n");
            fprintf (pFile, "-- Fin du inferieur strict\n");
		break;
		case LE :
            fprintf (pFile, "-- Il y a un inferieur ou egal\n");
            makeCode(getChild(tree, 0),env, pFile);
            makeCode(getChild(tree, 1),env, pFile);
            fprintf (pFile, "INFEQ\n");
            fprintf (pFile, "-- Fin du inferieur ou egal\n");
		break;
		case GT :
            fprintf (pFile, "-- Il y a un superieur strict\n");
            makeCode(getChild(tree, 0),env, pFile);
            makeCode(getChild(tree, 1),env, pFile);
            fprintf (pFile, "SUP\n");
            fprintf (pFile, "-- Fin du superieur strict\n");
		break;
		case GE :
            fprintf (pFile, "-- Il y a un superieur ou egal\n");
            makeCode(getChild(tree, 0),env, pFile);
            makeCode(getChild(tree, 1),env, pFile);
            fprintf (pFile, "SUPEQ\n");
            fprintf (pFile, "-- Fin du superieur ou egal\n");
		break;
		case AND : /* on utilise '&' pour les concatenations */
            fprintf (pFile, "-- Il y a une concatenation\n");
            makeCode(getChild(tree, 0),env, pFile);
            makeCode(getChild(tree, 1),env, pFile);
            fprintf (pFile, "CONCAT\n");
            fprintf (pFile, "-- Fin de la concatenation\n");
		break;
		case CST :
            fprintf (pFile, "PUSHI %d\n", tree->u.val);
		break;
		case E_SELECT :
            fprintf (pFile, "-- Il y a une selection (R)\n");
            makeCode(getChild(tree, 0),env, pFile);/* On ecrit le code donnant l'offset de l'expression */
            char* strAttr = getChild(tree, 1)->u.str;
            VarDeclP attrs = getChild(tree, 0)->u.lvar->coeur->_obj->attributes;
            fprintf(pFile, "LOAD %d\n", getOffsetAttr(attrs, strAttr));
            fprintf (pFile, "-- Fin de la selection (R)\n");
        break;
        case _ID :
            /* on empile l'offset de la variable ou de l'objet independant ayant cet identifiant */
            fprintf(pFile, "PUSHG %d\n", getOffsetObj(obj, tree->u.str));
        break;
        case I_AFF :
            fprintf (pFile, "-- Il y a une affectation\n");
            makeCodeAffect(getChild(tree, 0), getChild(tree, 1),env, pFile);
            fprintf (pFile, "-- Fin de l'affectation\n");
        break;
        case LIST_INST :
            makeCode(getChild(tree, 0),env, pFile);
            makeCode(getChild(tree, 1),env, pFile);
        break;
        case CAST :
            fprintf(pFile, "-- Il y a un cast\n" );
            /*je vois pas comment faire celui-l?*/
        break;
        case E_CALL_METHOD :
            fprintf(pFile, "-- Il y a un appel de methode\n");
            GcCallMethod(env,file,list,tree);
            makeCode(getChild(tree,0),env,file);
        break;
        case I_ITE :
            fprintf(pFile, "-- Il y a un bloc If Then Else\n");
            /*? c'est comme les tds/dm/exams ez*/
        break;
        case I_BLOC :
            fprintf(pFile, "-- Il y a un bloc d'instructions\n");
            /*plusieurs instructions a lire*/
                  int i;
            for(i=0;i<tree->nbChildren;i++){

            	makeCode(getChild(tree,i),env,pFile);
            }
        break;
        case I_RETURN :
        /*pas s?r s?r du truc*/
        break;
        case CLASS_NAME :
            /*dunno how to do dis Oo*/
        break;
        case _OVERRIDE :
            /*pas s?r s?r*/
        break;
		default :
            fprintf (pFile, "-- Il y a quelque chose\n");
		break;
	}
	fclose(file);
}

void makeCodeAffect(TreeP exprG, TreeP exprD,list_ClassObjP env, FILE* pFile) {

	switch(exprG->op) {
	    case E_SELECT :
            makeCode(getChild(exprG, 0), env,pFile);/* On ecrit le code donnant l'offset de l'expression */
            char* strAttr = getChild(exprG, 1)->u.str;
            VarDeclP attrs = getChild(exprG, 0)->u.lvar->coeur->_obj->attributes;
            makeCode(exprD,env, pFile);
            fprintf(pFile, "STORE %d\n", getOffsetAttr(attrs, strAttr));
        break;
        case _ID :
            ;
        break;
        default :
            fprintf (pFile, "-- On veut faire une affectation a autre chose qu'une variable ou un champ \n");
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
        printf("Objet introuvable\n");
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
        printf("Attribut introuvable\n");
    return i;
}

int getOffsetMeth(t_method* meth, char* nom){
	int offset = 0;
	while (meth != NULL && strcmp(meth->name, nom)){
		offset++;
		meth = meth->next;
	}
	if(meth == NULL)
		printf("Methode introuvable\n");
	return offset;
}

int getOffsetClass(t_class* class, char* nom){
	int offset = 0;
	while (class != NULL && strcmp(class->name,nom)){
		offset++;
		class = class->next;
	}
	if(class == NULL)
		printf("Classe introuvable\n");
	return offset;
}

void InitTV(list_ClassObjP env, FILE* pFile,t_method* list){

	if(env != NIL(list_ClassObj))
	{
		char* label = malloc(10*sizeof(char));
		int i = 0;
		fprintf(pFile, "-- Initilisation des tables virtuelles\n");
		fprintf(pFile, "init :");
		while(env->listClass->next != NIL(t_class)){

			fprintf(pFile, "\tALLOC %d\n",tailleAlloc(env->listClass->parametres));
			fprintf(pFile, "\t\tDUPN %d\n",1);

			while(env->listClass->methods->next != NIL(t_method)){

				if(env->listClass->methods->isRedef)
				{
					sprintf(label,"%c",'R');
					sprintf(label+1,"%d",i+1);
					sprintf(label+2,"%s",env->listClass->methods->name);
					i++;
				}
				else
				{
					sprintf(label,"%s",env->listClass->methods->name);
				}

				fprintf(pFile, "\t\tPUSHA %s\n",label);
				fprintf(pFile, "\t\tSTORE %d\n",getOffsetMeth(list,env->listClass->methods->name));

				if(env->listClass->methods->next != NIL(t_method))
				{
					fprintf(pFile, "\t\tDUPN %d\n",1);
				}
				env->listClass->methods = env->listClass->methods->next;
			}
			env->listClass = env->listClass->next;
		}

		fprintf(pFile, "-- Debut main\n");
		fprintf(pFile, "\t\tJUMP %s\n","main");
		free(label);
	}		
}

t_method* InitMethod(list_ClassObjP env, FILE* pFile){

	t_method* list = NEW(10,t_method);
	if (env != NIL(list_ClassObj))
	{
		char* label = malloc(10*sizeof(char));
		int i = 0;
		fprintf(pFile, "\t\tJUMP %s\n","init");
		fprintf(pFile, "-- Corps des methodes\n");
		while(env->listClass->next != NIL(t_class)){

			while(env->listClass->methods->next != NIL(t_method)){

				if(env->listClass->methods->isRedef)
				{
					sprintf(label,"%c",'R');
					sprintf(label+1,"%d",i+1);
					sprintf(label+2,"%s",env->listClass->methods->name);
					i++;
				}
				else
				{
					sprintf(label,"%s",env->listClass->methods->name);
					list->next = list;
					list = env->listClass->methods;
				}
				fprintf(pFile, "%s :\t",label);
				fprintf(pFile, "PUSHS %s\n",env->listClass->methods->name);
				fprintf(pFile, "\t\tWRITES\n");
				fprintf(pFile, "\t\tRETURN\n");
			
				env->listClass->methods = env->listClass->methods->next;
			}

			env->listClass = env->listClass->next;
		}

		free(label);
		return list;	
	}
	return NULL;
}

void CallMethod(list_ClassObjP env, FILE* pFile, t_method* list){

	if(env != NIL(list_ClassObj))
	{
		char* label = malloc(6*sizeof(char));
		sprintf(label,"%s","call");
		fprintf(pFile, "-- Appel des methodes\n");
		while(env->listClass->next != NIL(t_class)){

			while(env->listClass->methods->next != NIL(t_method)){

				if(!env->listClass->methods->isRedef)
				{
					sprintf(label+5,"%d",getOffsetMeth(list,env->listClass->methods->name));
					fprintf(pFile, "%s :\t",label);
					fprintf(pFile, "PUSHL %d\n",-1);
					fprintf(pFile, "\t\tDUPN %d\n",1);
					fprintf(pFile, "\t\tLOAD %d\n",0);
					fprintf(pFile, "\t\tLOAD %d\n",getOffsetMeth(list,env->listClass->methods->name));
					fprintf(pFile, "\t\tCALL\n");
					fprintf(pFile, "\t\tRETRUN\n");
				}
				env->listClass->methods = env->listClass->methods->next;
			}

			env->listClass = env->listClass->next;
		}

		free(label);
	}
}

void GcCallMethod(list_ClassObjP env,FILE* pFile,t_method* list,TreeP tree){
	char* label = malloc(6*sizeof(char));
	sprintf(label,"%s","call");
	t_class* classes = env->listClass;

	while(classes != NULL && strcmp(classes->methods->name,getChild(tree,1)->u.lvar->name)){
		classes->methods = classes->methods->next;

		if(classes->methods->next == NULL)
			classes = classes->next;
	}

	fprintf(pFile,"\t\tPUSHL %d\n",getOffsetClass(env->listClass,classes->name));
	sprintf(label+5,"%d",getOffsetMeth(list,classes->methods->name));
	fprintf(pFile, "\t\tPUSHA %s\n", label);
	fprintf(pFile, "\t\tCALL\n");
	fprintf(pFile, "\t\tPOPN %d\n",1);
}