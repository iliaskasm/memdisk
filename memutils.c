/*
 * Memdisk utilities
 *
 * Ilias K. Kasmeridis, 2018
 */

#include <stdio.h>
#include <string.h>
#include "memdisk.h"
#include <sys/stat.h>

char *tmpdir(char *s)
{
	char x[256];
	sprintf(x, "/home/pi/memdisk/files/%s", s);
	return strdup(x);
}

int findf(memfs_t *fs, char *filename)
{
	int i;
	int numfiles = 0;
	for (i=0; i<fs->size; i++)
	{
		char *fname;
		if (fs->recs[i].used == 0)
			continue;
		
		if (fs->recs[i].type == 1)
			fname = strdup(fs->files[i].m.filename);
		else if (fs->recs[i].type == 2)
			fname = strdup(fs->files[i].f.filename);

		if (!fname)
			continue;

		if (strcmp(filename, fname) == 0)
			return i;
	}
	return -1;
}

int isbin(memfs_t *fs, int x)
{
	int y;
	for (y=0; y<fs->files[x].m.size; y++)
	{
		if (fs->files[x].m.buffer[y] > 127)
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