/*
 * Memdisk header file
 * Compile with -lpthreads sharedmem.c memutils.c
 *
 * Ilias K. Kasmeridis, 2018
 */

#ifndef __MEMDISK_H__
#define __MEMDISK_H__

#include <sys/types.h>

#define Create O_RDWR | O_CREAT
#define Perm S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

#define fnex(x, filename)	int x; \
								x=findf(currdir(),filename); \
								if (x==-1) return -1;
#define fex(x, filename) int x; \
								x=findf(currdir(),filename); \
								if (x!=-1) return -1;

#define CMD(x) CMD_ ##x

#define currdir() sessions[currsessid].currdir
#define dir(x) currdir()->files[x].f
#define dirname(x) dir(x)->filename
#define dircb(x) dir(x)->cb
#define dirsize(x) dir(x)->size

#define fl(x) currdir()->files[x].m
#define flname(x) fl(x)->filename
#define flbuf(x) fl(x)->buffer
#define flcb(x) fl(x)->cb
#define flsize(x) fl(x)->size

#define mused(x) currdir()->recs[x].used /* 0 if memory slot unavailable, 1 if available */
#define mtype(x) currdir()->recs[x].type /* 1 for file, 2 for directory */

typedef enum {
	CMD(unknown) = -1, 
	CMD(list),  CMD(todisk), CMD(fromdisk), 
	CMD(rm), CMD(quota), CMD(mk),
	CMD(mkdir), CMD(cs), CMD(cd), 
	CMD(pwd), 
	CMD(lastcmd)
} cmdval_t;

typedef struct memfilecb {
	int64_t creation;
	int64_t modification;
	char *owner;
	unsigned int perms[9];
} memcb_t;

typedef struct memfile {
	char *filename;
	unsigned char *buffer;
	memcb_t cb;
	struct memfolder *parent;
	int size;
} memfile_t;

typedef struct record {
	int used;
	int type;
} memrecord_t;

typedef struct memfolder {
	char *filename;
	int size;
	struct memfolder *parent;
	memcb_t cb;
	union nodes {
		struct memfolder *f;
		struct memfile *m;
	} *files;
	memrecord_t *recs;
	int nfiles;
} memfolder_t;


typedef struct filesystem {
	memfolder_t init;

	int avail;
	int size;
	long int sizebytes;
} memfs_t;

typedef struct session {
	memfolder_t *currdir;
	char *id;
	int used;
} memsession_t;

#endif