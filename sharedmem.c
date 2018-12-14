/*
 * Shared memory source code
 *
 * Ilias K. Kasmeridis, 2018
 */

#include <stdio.h>
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
	pthread_mutexattr_init(&sharedmem->attrlock);
	pthread_mutexattr_setpshared(&sharedmem->attrlock, PTHREAD_PROCESS_SHARED);
	pthread_condattr_init(&sharedmem->attrcond);
	pthread_condattr_setpshared(&sharedmem->attrcond, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&(sharedmem->lock), &sharedmem->attrlock);
	pthread_cond_init(&(sharedmem->cond), &sharedmem->attrcond);
	sharedmem->haveread = 1;
	sharedmem->endofcmd = 0;
	sharedmem->value = RESETVAL;
	strcpy(sharedmem->arg1, "nop");
}

void sharedmem_reset(shmem_t *sharedmem)
{
	sharedmem->value = RESETVAL;
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
	return (sharedmem->value == RESETVAL);
}

void sharedmem_detach(shmem_t *sharedmem)
{
	shmdt(sharedmem); 
}

void sharedmem_destroy(shmem_t *sharedmem)
{
	
	pthread_cond_destroy(&(sharedmem->cond));
	pthread_mutex_destroy(&(sharedmem->lock));
	pthread_mutexattr_destroy(&sharedmem->attrlock);
	pthread_condattr_destroy(&sharedmem->attrcond);
	sharedmem_detach(sharedmem);
	shmctl(shmid,IPC_RMID,NULL); 
}