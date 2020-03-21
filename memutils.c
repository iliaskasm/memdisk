/*
 * Memdisk utilities
 *
 * Ilias K. Kasmeridis, 2018
 */

#include <stdio.h>
#include <string.h>
#include "memutils.h"
#include <sys/stat.h>

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
	for (i=0; i<folder->size; i++)
	{
		char *fname = NULL;
		// printf("%d\n", folder->recs[i].used);
		if (folder->recs[i].used == 0)
			continue;
		
		if (folder->recs[i].type == 1)
			fname = strdup(folder->files[i].m->filename);
		else if (folder->recs[i].type == 2)
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
	for (y=0; y<folder->files[x].m->size; y++)
	{
		if (folder->files[x].m->buffer[y] > 127)
			return 1;
	}
	return 0;
}

off_t fsize(const char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}