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

#define fl(x) fs.files[x].m
#define flname(x) fl(x).filename
#define flbuf(x) fl(x).buffer
#define flcb(x) fl(x).cb
#define flsize(x) fl(x).size

#define dir(x) fs.files[x].f
#define dirname(x) dir(x).filename
#define dirbuf(x) dir(x).buffer
#define dircb(x) dir(x).cb
#define dirsize(x) dir(x).size

#define mused(x) fs.recs[x].used
#define mtype(x) fs.recs[x].type

typedef struct memfilecb {
	int64_t creation;
	int64_t modification;
	char *owner;
	unsigned int perms[9];
} memcb_t;

typedef struct memfolder {
	char *filename;
	int size;
	struct memfolder *parent;
	memcb_t cb;
	union files {
		struct memfolder *f;
		struct memfile *file;
	} *children;
} memfolder_t;

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

typedef struct filesystem {
	union nodes {
		memfile_t m;
		memfolder_t f;
	} *files;
	memrecord_t *recs;
	int nfiles;
	int size;
	long int sizebytes;
} memfs_t;

typedef struct session {
	int id;
	char *currdir;
} memsession_t;

#endif