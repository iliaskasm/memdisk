/*
 * Shared memory between memdisk daemon and client
 *
 * Ilias K. Kasmeridis, 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include "sharedmem.h"

int shmid;

shmem_t *sharedmem_get(char *file, int size)
{
	key_t key = ftok(file, 65);
	shmid = shmget(key, size, 0666 | IPC_CREAT); 
	return (shmem_t*) shmat(shmid,(void*)0,0); 
}

void sharedmem_init(shmem_t *sharedmem)
{
	int i;

	pthread_mutexattr_init(&sharedmem->attrlock);
	pthread_mutexattr_setpshared(&sharedmem->attrlock, PTHREAD_PROCESS_SHARED);
	pthread_condattr_init(&sharedmem->attrcond);
	pthread_condattr_setpshared(&sharedmem->attrcond, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&(sharedmem->lock), &sharedmem->attrlock);
	pthread_cond_init(&(sharedmem->cond), &sharedmem->attrcond);

	sharedmem->nargs = 0;
	sharedmem->haveread = 1;
	sharedmem->endofcmd = 0;
	strcpy(sharedmem->value, RESETVAL);
	strcpy(sharedmem->args[0], "nop");
}

void sharedmem_reset(shmem_t *sharedmem)
{
	int i;

	strcpy(sharedmem->value, RESETVAL);
	for (i=0; i<sharedmem->nargs; i++)
	{
		strcpy(sharedmem->args[i], "");
	}
	sharedmem->nargs = 0;
	// strcpy(sharedmem->cmd, "nop");
	// memset(sharedmem->response,0,sizeof(sharedmem->response));
}

void sharedmem_lock(shmem_t *sharedmem)
{
	pthread_mutex_lock(&(sharedmem->lock));
}

void sharedmem_unlock(shmem_t *sharedmem)
{
	pthread_mutex_unlock(&(sharedmem->lock));
}

void sharedmem_wait(shmem_t *sharedmem)
{
	pthread_cond_wait(&(sharedmem->cond), &(sharedmem->lock));
}

void sharedmem_signal(shmem_t *sharedmem)
{
	pthread_cond_signal(&(sharedmem->cond));
}

int sharedmem_isempty(shmem_t *sharedmem)
{
	return (strcmp(sharedmem->value, RESETVAL) == 0);
}

void sharedmem_detach(shmem_t *sharedmem)
{
	shmdt(sharedmem); 
}

void sharedmem_destroy(shmem_t *sharedmem)
{
	int i;

	pthread_cond_destroy(&(sharedmem->cond));
	pthread_mutex_destroy(&(sharedmem->lock));
	pthread_mutexattr_destroy(&sharedmem->attrlock);
	pthread_condattr_destroy(&sharedmem->attrcond);
	sharedmem_detach(sharedmem);
	shmctl(shmid,IPC_RMID,NULL); 

	for (i=0; i<sharedmem->nargs; i++)
	{
		strcpy(sharedmem->args[i], "");
	}
	sharedmem->nargs = 0;
}