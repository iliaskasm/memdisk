/*
 * Memdisk daemon
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
#include "memutils.h"

struct timeval currtime;
shmem_t *shared_mem;
memsession_t *sessions;
int nsessions = 0;
int currsessid = 0;
memfs_t fs;

char *cmdstrings[] = {
	"ls", "to", "from", "rm", "quota",
	"touch", "mkdir", "cs", "cd", "pwd"
};

int write_to_file(char *filename, void *buf, int bytes)
{
	int status;
	CHECK_IF_NOT_EXISTS(x,filename);

	if (memfilesize(x) > 0 && memfilesize(x) < bytes)
		memfilebuf(x) = (unsigned char*) realloc(memfilebuf(x), bytes * sizeof(unsigned char));
	else 
		memfilebuf(x) = (unsigned char*) malloc(bytes * sizeof(unsigned char));
	
	memfilesize(x) = bytes;
	fs.sizebytes += bytes;
	memcpy(memfilebuf(x), buf, bytes);
	gettimeofday(&currtime, NULL);
	memfilecb(x).modification = (currtime.tv_sec * INT64_C(1000)) + (currtime.tv_usec / 1000);

	return 1;
}

int memdisk_mkdir(char *filename)
{
	int status;
	int i, index=0;

	CHECK_IF_EXISTS(x,filename);

	/* Find the first available slot for the file */
	for (i=0; i<currdir()->size; i++)
	{
		if (memslotused(i) == MEMRECORD_SLOT_UNUSED)
		{
			index = i;
			// dirsize(i+1) = -1;
			memslotused(i+1) = MEMRECORD_SLOT_UNUSED;
			break;
		}
	}

	/* New directory initialization */
	memdir(index) = (memfolder_t*) malloc(sizeof(memfolder_t));

	if (memdir(index) != &(fs.init))
		memdir(index)->files = (union nodes*) malloc(MEMDIR_DEFAULT_SIZE * sizeof(union nodes));

	memdir(index)->records = (memrecord_t*) malloc(MEMDIR_DEFAULT_SIZE * sizeof(memrecord_t));
	memdir(index)->nfiles = 0;
	memdir(index)->parent = currdir();

	/* Instead of initializing every file size, we just do
	  it for the first one. As the table grows, we initialize
	  the next files.*/
	memdir(index)->records[0].used = MEMRECORD_SLOT_UNUSED;
	memdir(index)->records[0].type = MEMRECORD_UNINITIALIZED_TYPE;

	/* 
	 * The order of the operations matters. 
	 * We need to assure that the file accessed
	 * is a memdir and not a memfile. 
	 */
	memdirsize(index) = MEMDIR_DEFAULT_SIZE;
	memslotused(index) = MEMRECORD_SLOT_USED;
	memslottype(index) = MEMRECORD_DIR_TYPE;
	memdirname(index) = strndup(filename, 256);

	gettimeofday(&currtime, NULL);
	memdircb(index).creation = (currtime.tv_sec * INT64_C(1000)) + (currtime.tv_usec / 1000);
	memdircb(index).modification = (currtime.tv_sec * INT64_C(1000)) + (currtime.tv_usec / 1000);
	memdircb(index).fmt_modification_date = (char*) malloc(32 * sizeof(char));
	format_timeval(&currtime, memdircb(index).fmt_modification_date, 32);
	currdir()->nfiles++;

	// fs.sizebytes += 4096 * sizeof(union nodes) + sizeof(memfolder_t);
	fs.sizebytes += MEMDIR_DEFAULT_SIZE;
}

int memdisk_touch(char *filename)
{	
	int status;
	int i, index=0;

	CHECK_IF_EXISTS(x,filename);

	/* Find the first available slot for the file */
	for (i=0; i<currdir()->size; i++)
	{
		if (memslotused(i) == MEMRECORD_SLOT_UNUSED)
		{
			index = i;
			memslotused(i+1) = MEMRECORD_SLOT_UNUSED;
			break;
		}
	}

	/* New file initialization */
	memfile(index) = (memfile_t*) malloc(sizeof(memfile_t));

	/* 
	 * The order of the operations matters. 
	 * We need to assure that the file accessed
	 * is a memfile and not a memdir. 
	 */
	memfilesize(index) = 0;
	memslotused(index) = MEMRECORD_SLOT_USED;
	memslottype(index) = MEMRECORD_FILE_TYPE;
	memfilename(index) = strndup(filename, 256);

	gettimeofday(&currtime, NULL);
	memfilecb(index).creation = (currtime.tv_sec * INT64_C(1000)) + (currtime.tv_usec / 1000);
	memfilecb(index).modification = (currtime.tv_sec * INT64_C(1000)) + (currtime.tv_usec / 1000);
	memfilecb(index).fmt_modification_date = (char*) malloc(32 * sizeof(char));
	format_timeval(&currtime, memfilecb(index).fmt_modification_date, 32);
	currdir()->nfiles++;

	// fs.sizebytes += sizeof(memfile_t);
	return 1;
}

int memdisk_rm(char *filename)
{
	int status;
	int size;

	CHECK_IF_NOT_EXISTS(x,filename);
	memslotused(x) = MEMRECORD_SLOT_UNUSED;
	
	if (memslottype(x) == MEMRECORD_FILE_TYPE)
	{
		// size = sizeof(memfile_t);
		size = memfilesize(x);
		free(memfilename(x));
		if (memfilebuf(x))
			free(memfilebuf(x));
	}
	else if (memslottype(x) == MEMRECORD_DIR_TYPE)
	{
		// size = (memdirsize(x) * sizeof(union nodes)) + sizeof(memfolder_t);
		size = MEMDIR_DEFAULT_SIZE;
		free(memdirname(x));
		/* TODO: Check for files inside */
	}
	
	memmove(currdir()->files+x, currdir()->files+x+1, currdir()->size-x);
	memmove(currdir()->records+x, currdir()->records+x+1, currdir()->size-x);
	currdir()->nfiles--;
	fs.sizebytes -= size;
}

void memdisk_init(int bytes)
{
	int size = bytes / sizeof(union nodes);
	int i;

	fs.size = bytes - MEMDIR_DEFAULT_SIZE;
	fs.sizebytes = 0;
	fs.avail = bytes;
	// fs.files = (union nodes*) malloc(size * sizeof(union nodes));
	fs.init.filename = strdup("init");
	fs.init.size = MEMDIR_DEFAULT_SIZE;
	fs.init.files = (union nodes*) malloc(MEMDIR_DEFAULT_SIZE * sizeof(union nodes));
	
	sessions = (memsession_t*) malloc(MEMSESSION_NUMBANKS * sizeof(memsession_t));
	for (i=0; i<MEMSESSION_NUMBANKS; i++)
	{
		sessions[i].id = NULL;
		sessions[i].currdir = &(fs.init);
	}
	sessions[currsessid].currdir = &(fs.init);
	sessions[currsessid].currdir->records = (memrecord_t*) malloc(size * sizeof(memrecord_t));
	currdir()->nfiles = 0;


	/* Instead of initializing every file size, we just do
	  it for the first one. As the table grows, we initialize
	  the next files. */
	memslotused(0) = MEMRECORD_SLOT_UNUSED;
	memslottype(0) = MEMRECORD_UNINITIALIZED_TYPE;
}

void memdisk_destroy()
{
	int i;
	for (i=0; i<currdir()->size; i++)
		if (memslottype(i) == MEMRECORD_FILE_TYPE)
			free(memfilebuf(i));
	free(currdir()->files);
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
	write_to_file(destination, buf, filesize);
	free(buf);
}

int memdisk_todisk(char *source, char *destination)
{
	int status;
	CHECK_IF_NOT_EXISTS(x,source);

	if (isbin(currdir(), x))
	{
		int fd = open(destination, Create | O_TRUNC, Perm);
		if (memfilesize(x) > 0)
			write(fd, memfilebuf(x), memfilesize(x));
		close(fd);
	}
	else
	{
		FILE *fp = fopen(destination, "w");
		if (memfilesize(x) > 0)
			fprintf(fp, "%s", memfilebuf(x));
		fclose(fp);
	}
}

int memdisk_quota()
{
	char response[1024];
	snprintf(shared_mem->response, 1024, "%d\n",
		(fs.avail - fs.sizebytes));
}

void memdisk_list()
{
	int i;
	char datestring[32];

	while (shared_mem->haveread == 0)
		sh_wait();

	snprintf(shared_mem->response, 1023, "d 4096 .\nd 4096 ..\n");
	shared_mem->haveread = 0;
	sh_signal();

	for (i=0; i<currdir()->nfiles; i++)
	{
		while (shared_mem->haveread == 0)
			sh_wait();

		if (memfilesize(i) == -1)
			continue;

		printf("%d %d %d\n", memslottype(i), currdir()->nfiles, i);
		if (memslottype(i) == 1)
		{
			snprintf(shared_mem->response, 1023, "- [%p]"
			" "
			"%s "
			" "
			"%6d"
			" "
			"%s\n", 
			 &(memfile(i)), memfilecb(i).fmt_modification_date, memfilesize(i), memfilename(i));
		}
		else 
		{
			snprintf(shared_mem->response, 1023, "d [%p]"
			" "
			"%s "
			" "
			"%6d"
			" "
			"%s\n", 
			 &(memdir(i)), memdircb(i).fmt_modification_date, memdirsize(i), memdirname(i));
		}

		printf("%s", shared_mem->response);
		shared_mem->haveread = 0;
		sh_signal();
	}
}

void memdisk_pwd() 
{
	memfolder_t *currdir = sessions[currsessid].currdir;
	if (currdir == NULL)
	{
		snprintf(shared_mem->response, 1023, "/\n");
		return;
	}
	snprintf(shared_mem->response, 1023, "%s\n", currdir->filename);
}

int memdisk_cd(char *dir)
{
	int status;
	if (strcmp(dir, ".") == 0)
		return 0;

	if (strcmp(dir, "..") == 0)
	{
		if (sessions[currsessid].currdir->parent)
			sessions[currsessid].currdir = sessions[currsessid].currdir->parent;
	}
	else
	{
		CHECK_IF_NOT_EXISTS(x, dir);
		if (memslottype(x) == 1)
		{
			snprintf(shared_mem->response, 1023, "Not a directory.\n");
			return -1;
		}

		if (strcmp(dir, "..") == 0)
		{
			sessions[currsessid].currdir = sessions[currsessid].currdir->parent;
		}
		else
		{
			sessions[currsessid].currdir = memdir(x);
		}
	}
	
	
	snprintf(shared_mem->response, 1023, "Changed dir: %s\n", sessions[currsessid].currdir->filename);
	return 0;
}

int memdisk_cs(int sessid)
{
	if (sessid>MEMSESSION_NUMBANKS-1 || sessid<0)
	{
		snprintf(shared_mem->response, 1023, "Membank ID must be between 0 and %d.\n", MEMSESSION_NUMBANKS-1);
		return -1;
	}
	currsessid = sessid;
	snprintf(shared_mem->response, 1023, "Changed to membank %d\n", currsessid);
	return currsessid;
}

void handle(char *command)
{	
	int i, msg;
	printf("Data read from memory: %s\n", command); 
	for (i=0; i<shared_mem->nargs; i++)
	{
		printf("\t %s\n", shared_mem->args[i]);
	}
	memset(shared_mem->response,0,sizeof(shared_mem->response));

	msg = cmd_to_int(command);
	switch(msg) 
	{
		case CMD(list):
			memdisk_list();
			shared_mem->haveread = 0;
			break;
		case CMD(todisk):
			memdisk_todisk(shared_mem->args[0], shared_mem->args[1]);
			shared_mem->haveread = 0;
			break;
		case CMD(fromdisk):
			memdisk_fromdisk(shared_mem->args[0], shared_mem->args[1]);
			shared_mem->haveread = 0;
			break;
		case CMD(rm):
			memdisk_rm(shared_mem->args[0]);
			shared_mem->haveread = 0;
			break;
		case CMD(quota):
			memdisk_quota();
			shared_mem->haveread = 0;
			break;
		case CMD(mk):
			memdisk_touch(shared_mem->args[0]);
			shared_mem->haveread = 0;
			break;
		case CMD(mkdir):
			memdisk_mkdir(shared_mem->args[0]);
			shared_mem->haveread = 0;
			break;
		case CMD(cs):
			memdisk_cs(atoi(shared_mem->args[0]));
			shared_mem->haveread = 0;
			break;
		case CMD(cd):
			memdisk_cd(shared_mem->args[0]);
			shared_mem->haveread = 0;
			break;
		case CMD(pwd):
			memdisk_pwd();
			shared_mem->haveread = 0;
			break;
		default:
			break;
	}
}

int main(int argc, char *argv[])
{
	char cmd[16];
	int size = atoi(argv[1]);

	if (argc < 3)
		exit(1);

	if (strcmp(argv[2], "KB") == 0)
		size*=1000;
	else if (strcmp(argv[2], "MB") == 0)
		size*=1000000;
	else if (strcmp(argv[2], "GB") == 0)
		size*=1000000000;
	else
		exit(1);
	// fclose(stdout);

	printf("Initializing filesystem... ");
	memdisk_init(size);
	printf("OK\n");

	printf("Started memdisk with memory size: %s %s\n", argv[1], argv[2]);

	shared_mem = (shmem_t*) sh_get();
	sh_init();
	sh_lock();
	while (1)
	{	
		if (strcmp(cmd, EXITVAL) == 0)
			break;

		while (sh_isempty())
			sh_wait();

		strcpy(cmd, shared_mem->command);

		shared_mem->endofcmd = 0;
		handle(cmd);
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
