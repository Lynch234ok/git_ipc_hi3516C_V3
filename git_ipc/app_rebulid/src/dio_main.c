
#include "direct_io.h"


#include <stdlib.h>
#include <sys/types.h>  
#include <sys/stat.h>   
#include <fcntl.h>




#include "generic.h"
#include "frank_trace.h"

#include <assert.h>

#include "app_debug.h"
#ifdef _DIRECTIO_MAIN


int main(int argc, char *argv[])
{
	int copy_size = 0;
	unsigned char * buffer = NULL;
	int read_size = 0;
	LP_DIRECT_IO_HDC  dIO_read_fd = NULL;
	LP_DIRECT_IO_HDC  dIO_write_fd = NULL;

	char file_path1[128] = "dioTest.txt";
	char file_path2[128] = "dioTest2.txt";
	
	printf("%d\n",__LINE__);
	
	int cacheMax = 4096 *10;   


	dIO_read_fd = direct_io_open(file_path1, cacheMax, 0);    // read

	
	if(dIO_read_fd == NULL){
		printf("%d\n",__LINE__);
		}
	printf("%d\n",__LINE__);

	
	
	dIO_write_fd = direct_io_open(file_path2, cacheMax, 1);    // write


	if(dIO_write_fd == NULL){
		printf("%d\n",__LINE__);
		}
	printf("%d\n",__LINE__);



	do
	{
		copy_size = rand() % 1000 + 1024;

		
		printf("main**%d***copy_size= %ld\n",__LINE__,copy_size); 
		
		buffer = calloc(1,copy_size);

		if(buffer == NULL){                       //Test
			printf("calloc erro!%d",__LINE__);
		}

		
		read_size = direct_io_fread(dIO_read_fd,buffer,copy_size);

		printf("%d****read_size = %d\n",__LINE__,read_size);//Test
		
		if(read_size > 0){
			direct_io_fwrite(dIO_write_fd,buffer,read_size);
		}
		free(buffer);
	}while(read_size > 0);




	copy_size = rand() % 1000 + 1024;

	buffer = calloc(1, copy_size);
	lseek(dIO_read_fd, 1024, SEEK_SET);
	read(dIO_read_fd, buffer, copy_size);
	direct_io_fseek(dIO_write_fd, 1024, SEEK_SET);
	direct_io_fwrite(dIO_write_fd, buffer, copy_size);
	free(buffer);

	direct_io_close(dIO_read_fd);
	direct_io_close(dIO_write_fd);

	return 0;

}
#endif
