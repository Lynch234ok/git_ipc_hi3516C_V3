#include <stdio.h>
#include <stdlib.h>

#include "demo.h"

int main(int argc, char** argv)
{
	setlinebuf(stdout);
	setlinebuf(stdin);
	setvbuf(stderr,(char *)NULL,_IONBF,0);
	//
	return Demo_main(argc, argv);
}

