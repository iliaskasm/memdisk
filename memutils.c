/*
 * Memdisk utilities
 *
 * Ilias K. Kasmeridis, 2018
 */

#include <stdio.h>
#include <string.h>
#include "memutils.h"
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>


char *tmpdir(char *s)
{
	char x[256];
	sprintf(x, "/home/pi/memdisk/files/%s", s);
	return strdup(x);
}

int findf(memfolder_t *folder, char *filename)
{
	int i;
	int numfiles = 0;
	for (i = 0; i < folder->size; i++)
	{
		char *fname = NULL;
		// printf("%d\n", folder->records[i].used);
		if (folder->records[i].used == MEMRECORD_SLOT_UNUSED)
			continue;
		
		if (folder->records[i].type == MEMRECORD_FILE_TYPE)
			fname = strdup(folder->files[i].m->filename);
		else if (folder->records[i].type == MEMRECORD_DIR_TYPE)
			fname = strdup(folder->files[i].f->filename);

		if (!fname)
			continue;

		if (strcmp(filename, fname) == 0)
			return i;
	}
	return -1;
}

int isbin(memfolder_t *folder, int x)
{
	int y;
	for (y = 0; y < folder->files[x].m->size; y++)
	{
		if (folder->files[x].m->buffer[y] > 127)
			return 1;
	}
	return 0;
}

off_t fsize(const char *filename) 
{
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}

int cmd_to_int(char *command)
{
	int i;
	for (i = 0; i < CMD(lastcmd); i++)
	{
		if (strcmp(command, cmdstrings[i]) == 0)
		{
			return i;
		}
	}

	return CMD(unknown);
}

ssize_t format_timeval(struct timeval *tv, char *buf, size_t sz)
{
	ssize_t written = -1;
	struct tm *gm = gmtime(&tv->tv_sec);
	int w;

	if (gm)
	{
		written = (ssize_t)strftime(buf, sz, "%b %d\t%H:%M", gm);
	}
	return written;
}