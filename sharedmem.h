/*
 * Shared memory header file
 *
 * Ilias K. Kasmeridis, 2018
 */

#ifndef __SHAREDMEM_H__
#define __SHAREDMEM_H__

#include <pthread.h>



#define EXITVAL		500
#define RESETVAL	-1
#define SHMEMSIZE	131072000

typedef struct shmem {
	char **argv;
	int value;
	char arg1[128];
	char response[1024];
	int endofcmd;
	int haveread;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	pthread_condattr_t attrcond;
	pthread_mutexattr_t attrlock;
} shmem_t;

shmem_t *sharedmem_get(char *file, int size);
void sharedmem_init(shmem_t *sharedmem);
void sharedmem_reset(shmem_t *sharedmem);
void sharedmem_lock(shmem_t *sharedmem);
void sharedmem_unlock(shmem_t *sharedmem);
void sharedmem_wait(shmem_t *sharedmem);
void sharedmem_signal(shmem_t *sharedmem);
int  sharedmem_isempty(shmem_t *sharedmem);
void sharedmem_detach(shmem_t *sharedmem);
void sharedmem_destroy(shmem_t *sharedmem);

#define sh_get() 		sharedmem_get("shmfile", SHMEMSIZE)
#define sh_init() 		sharedmem_init(shared_mem)
#define sh_reset() 		sharedmem_reset(shared_mem)
#define sh_lock()		sharedmem_lock(shared_mem)
#define sh_unlock()		sharedmem_unlock(shared_mem)
#define sh_wait()		sharedmem_wait(shared_mem)
#define sh_signal()		sharedmem_signal(shared_mem)
#define sh_isempty()	sharedmem_isempty(shared_mem)
#define sh_detach()		sharedmem_detach(shared_mem)
#define sh_destroy()	sharedmem_destroy(shared_mem)

#endif