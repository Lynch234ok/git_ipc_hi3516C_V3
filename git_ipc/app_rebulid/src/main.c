#include <stdio.h>
#include <stdlib.h>

#include "ipcam.h"
#include "base/ja_process.h"

int main(int argc, char** argv)
{
	setlinebuf(stdout);
	setlinebuf(stdin);
	setvbuf(stderr,(char *)NULL,_IONBF,0);
	NK_PROCESS_init();
	//
	return IPCAM_main(argc, argv);
}

