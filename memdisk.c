/*
 * Memdisk source code
 * Compile with -lpthread sharedmem.c memutils.c
 *
 * Ilias K. Kasmeridis, 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/ipc.h> 
#include "sharedmem.h"
#include <pthread.h>
#include "memdisk.h"

struct timeval currtime;
shmem_t *shared_mem;
memsession_t *sessions;
int currsessid;
memfs_t fs;

int memdisk_mkdir(char *filename)
{
	int status;
	fex(x,filename);

	/* Find the first available slot for the file */
	int i, index=0;
	for (i=0; i<fs.size; i++)
	{
		if (mused(i) == 0)
		{
			index = i;
			// dirsize(i+1) = -1;
			mused(i+1) = 0;
			break;
		}
	}

	/* 
	 * The order of the operations matters. 
	 * We need to assure that the file accessed
	 * is a memdir and not a memfile. 
	 */
	dirsize(index) = 0;
	mused(index) = 1;
	mtype(index) = 2;
	dirname(index) = strndup(filename, 256);

	gettimeofday(&currtime, NULL);
	dircb(index).creation = (currtime.tv_sec * INT64_C(1000)) + (currtime.tv_usec / 1000);
	dircb(index).modification = (currtime.tv_sec * INT64_C(1000)) + (currtime.tv_usec / 1000);
	fs.nfiles++;
}

int memdisk_touch(char *filename)
{	
	int status;
	fex(x,filename);

	/* Find the first available slot for the file */
	int i, index=0;
	for (i=0; i<fs.size; i++)
	{
		if (mused(i) == 0)
		{
			printf("gonna write to %d\n", i);
			index = i;
			mused(i+1) = 0;
			break;
		}
	}

	/* 
	 * The order of the operations matters. 
	 * We need to assure that the file accessed
	 * is a memfile and not a memdir. 
	 */
	flsize(index) = 0;
	mused(index) = 1;
	mtype(index) = 1;
	flname(index) = strndup(filename, 256);

	gettimeofday(&currtime, NULL);
	flcb(index).creation = (currtime.tv_sec * INT64_C(1000)) + (currtime.tv_usec / 1000);
	flcb(index).modification = (currtime.tv_sec * INT64_C(1000)) + (currtime.tv_usec / 1000);
	fs.nfiles++;

	return 1;
}

int memdisk_rm(char *filename)
{
	int status;
	fnex(x,filename);

	mused(x) = 0;

	if (mtype(x) == 1)
	{
		free(flname(x));
		if (flbuf(x))
			free(flbuf(x));
	}
	else if (mtype(x) == 2)
	{
		free(dirname(x));
		/* TODO: Check for files inside */
	}
	
	memmove(fs.files+x, fs.files+x+1, fs.size-x);
	memmove(fs.recs+x, fs.recs+x+1, fs.size-x);
	fs.nfiles--;
}

int memdisk_read(char *filename, void *buf, int bytes)
{
	int status;
	fnex(x,filename);

	memcpy(buf, flbuf(x), bytes);
	return 1;
}

int memdisk_write(char *filename, void *buf, int bytes)
{
	int status;
	fnex(x,filename);

	if (flsize(x) > 0 && flsize(x) < bytes)
		flbuf(x) = (unsigned char*) realloc(flbuf(x), bytes * sizeof(unsigned char));
	else 
		flbuf(x) = (unsigned char*) malloc(bytes * sizeof(unsigned char));
	
	flsize(x) = bytes;
	fs.sizebytes += bytes;
	memcpy(flbuf(x), buf, bytes);
	gettimeofday(&currtime, NULL);
	flcb(x).modification = (currtime.tv_sec * INT64_C(1000)) + (currtime.tv_usec / 1000);

	return 1;
}

void memdisk_init(int bytes)
{
	int size = bytes / sizeof(union nodes);
	fs.size = size;
	fs.sizebytes = 0;
	fs.nfiles = 0;
	fs.files = (union nodes*) malloc(size * sizeof(union nodes));
	fs.recs = (memrecord_t*) malloc(size * sizeof(memrecord_t));
	int i;

	/* Instead of initializing every file size, we just do
	  it for the first one. As the table grows, we initialize
	  the next files.*/
	mused(0) = 0;
	mtype(0) = 0;

	sessions = (memsession_t*) malloc(10*sizeof(memsession_t));
	for (i=0; i<10; i++)
	{
		sessions[i].id = -1;
		sessions[i].currdir = (char*) malloc(256*sizeof(char));
		strcpy(sessions[i].currdir, "");
	}
}

void memdisk_destroy()
{
	int i;
	for (i=0; i<fs.size; i++)
		if (flbuf(i))
			free(flbuf(i));
	free(fs.files);
}

int memdisk_fromdisk(char *source, char *destination)
{
	int i = 0;
	int n;
	int filesize = fsize(source);
	unsigned char *buf = (unsigned char*) malloc(filesize * sizeof(unsigned char));

	int fd = open(source, O_RDONLY);
	while ((n=read(fd, buf, filesize)) > 0)
	{
		// buf[i++] = y;
		// printf("iteration %d\n", i);
	}
	close(fd);

	memdisk_touch(destination);
	memdisk_write(destination, buf, filesize);
	free(buf);
}

int memdisk_todisk(char *source, char *destination)
{
	int status;
	fnex(x,source);

	if (isbin(&fs, x))
	{
		int fd = open(destination, Create | O_TRUNC, Perm);
		if (flsize(x) > 0)
			write(fd, flbuf(x), flsize(x));
		close(fd);
	}
	else
	{
		FILE *fp = fopen(destination, "w");
		if (flsize(x) > 0)
			fprintf(fp, "%s", flbuf(x));
		fclose(fp);
	}
}

int memdisk_quota()
{
	char response[1024];
	snprintf(shared_mem->response, 1024, "%d\n",
		(fs.size-fs.nfiles)*sizeof(memfile_t) - fs.sizebytes);
	shared_mem->haveread = 0;
}

void memdisk_list()
{
	int i;
	for (i=0; i<fs.nfiles; i++)
	{
		while (shared_mem->haveread == 0)
			sh_wait();

		if (flsize(i) == -1)
			continue;

		printf("%d %d %d\n", mtype(i), fs.nfiles, i);
		if (mtype(i) == 1)
		{
			snprintf(shared_mem->response, 1023, "- [%p]"
			" "
			"%" PRId64 ""
			" "
			"%6d"
			" "
			"%s\n", 
			 &(fl(i)), flcb(i).modification, flsize(i), flname(i));
		}
		else 
		{
			snprintf(shared_mem->response, 1023, "d [%p]"
			" "
			"%" PRId64 ""
			" "
			"%6d"
			" "
			"%s\n", 
			 &(dir(i)), dircb(i).modification, flsize(i), dirname(i));
		}

		printf("%s", shared_mem->response);
		shared_mem->haveread = 0;
		sh_signal();
	}
}

void handle(int msg)
{	
	printf("Data read from memory: %d %s\n", msg, shared_mem->arg1); 
	memset(shared_mem->response,0,sizeof(shared_mem->response));
	switch(msg) 
	{
		case CMD(list):
			memdisk_list();
			shared_mem->haveread = 0;
			break;
		case CMD(write):
			break;
		case CMD(read): 
			break;
		case CMD(todisk):
			break;
		case CMD(fromdisk):
			break;
		case CMD(rm):
			memdisk_rm(shared_mem->arg1);
			shared_mem->haveread = 0;
			break;
		case CMD(quota):
			memdisk_quota();
			shared_mem->haveread = 0;
			break;
		case CMD(mk):
			memdisk_touch(shared_mem->arg1);
			shared_mem->haveread = 0;
			break;
		case CMD(mkdir):
			memdisk_mkdir(shared_mem->arg1);
			shared_mem->haveread = 0;
			break;
		default:
			break;
	}
}

int main(int argc, char *argv[])
{

	if (argc < 3)
		exit(1);

	int size = atoi(argv[1]);
	if (strcmp(argv[2], "KB") == 0)
		size*=1000;
	else if (strcmp(argv[2], "MB") == 0)
		size*=1000000;
	else if (strcmp(argv[2], "GB") == 0)
		size*=1000000000;
	else
		exit(1);
	fclose(stdout);
	printf("Initializing filesystem... ");
	memdisk_init(size);
	printf("OK\n");

	int val;

	shared_mem = (shmem_t*) sh_get();
	sh_init();
    sh_lock();
    while (1)
    {	
    	if (val == EXITVAL)
    		break;

    	while (sh_isempty())
    		sh_wait();


    	val = shared_mem->value;

    	shared_mem->endofcmd = 0;
    	handle(val);
    	shared_mem->endofcmd = 1;
    	
    	sh_reset();
    	sh_signal();
    }

    shared_mem->haveread = 0;
    shared_mem->endofcmd = 1;
    sh_signal();

    sh_unlock();
    sh_destroy();

    memdisk_destroy();
    return 0; 
}
