#include <stdio.h>
#include "tp.h"
#include "tp_y.h"
#include <fcntl.h>

void makeCode();


extern char* strdup(const char *);


/* int main(int argc, char **argv) {
	
	printf("Ecriture du code\n");
	
	makeCode();
	
	
	return 0;
} */




void makeCode() {/*TreeP* tree*/
	
	FILE * pFile;
	pFile = fopen ("myfile.txt","w");
	
	if (pFile!=NULL) {
		fputs ("fopen example",pFile);
		fclose (pFile);
	}
	
	/*switch(op) {
		case SUM :
		
		break;
	}*/
	
}
