/*
 * Memdisk client
 * Compile with -lpthreads sharedmem.c
 *
 * Ilias K. Kasmeridis, 2018
 */

#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <unistd.h> 
#include "sharedmem.h"

shmem_t *shared_mem;

int main(int argc, char *argv[]) 
{ 
	if (argc != 3)
		return 0;

    shared_mem = (shmem_t*) sh_get();
    sh_lock();

    strcpy(shared_mem->cmd, argv[2]);
    shared_mem->cmd[strlen(shared_mem->cmd)] = 0;
    shared_mem->value = atoi(argv[1]);
    sh_signal();
    
    while (1)
    {	
    	if (atoi(argv[1]) == 500)
    		break;
    	while (shared_mem->haveread == 1)
    		sh_wait();

    	if (strcmp(shared_mem->response, ""))
    		printf("%s", shared_mem->response);

    	shared_mem->haveread = 1;
    	sh_signal();

    	
    	if (shared_mem->endofcmd)
    		break;
    }

    sh_unlock();
    sh_detach();

    return 0; 
} 