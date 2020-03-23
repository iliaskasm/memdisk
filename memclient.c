/*
 * Memdisk client
 *
 * Ilias K. Kasmeridis, 2018
 */

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
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
    int i;
	if (argc < 2)
    {
        printf("Syntax: %s <command> [args...]\n");
		return 0;
    }

    shared_mem = (shmem_t*) sh_get();
    sh_lock();

    shared_mem->nargs = argc - 2;

    for (i=0; i<shared_mem->nargs; i++)
    {
        strcpy(shared_mem->args[i], argv[2+i]);
    }


    // strcpy(shared_mem->arg1, argv[2]);
    // shared_mem->arg1[strlen(shared_mem->arg1)] = 0;

    // if (argc==4)
    // {
    //     strcpy(shared_mem->arg2, argv[3]);
    //     shared_mem->arg2[strlen(shared_mem->arg2)] = 0;
    // }

    strcpy(shared_mem->value, argv[1]);

    sh_signal();  
    while (1)
    {	
    	if (strcmp(argv[1], "exit") == 0)
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
