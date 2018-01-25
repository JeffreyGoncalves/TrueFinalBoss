#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	if(argc > 1)
	{
		int i = 0, max = 100000000;
		float p = 0, ratio = (float)(max)/100.f;
		while(i < max)
		{
			p = (float)(i)/ratio;
			printf("%s %f %\r", argv[1], p);
			i++;
		}
	}
	else printf("Usage : ./ttt phrase \n");
	return 0;
}
