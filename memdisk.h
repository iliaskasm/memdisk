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

#define MEMRECORD_SLOT_UNUSED			0
#define MEMRECORD_SLOT_USED				1
#define MEMRECORD_UNINITIALIZED_TYPE	0
#define MEMRECORD_FILE_TYPE				1
#define MEMRECORD_DIR_TYPE				2

#define MEMDIR_DEFAULT_SIZE				4096
#define MEMSESSION_NUMBANKS 			11

#define CHECK_IF_NOT_EXISTS(x, filename)	int x = findf(currdir(),filename); \
								if (x==-1) return -1;
#define CHECK_IF_EXISTS(x, filename) int x = findf(currdir(),filename); \
								if (x!=-1) return -1;

#define CMD(x) CMD_ ##x

#define currdir() sessions[currsessid].currdir
#define memdir(x) currdir()->files[x].f
#define memdirname(x) memdir(x)->filename
#define memdircb(x) memdir(x)->cb
#define memdirsize(x) memdir(x)->size

#define memfile(x) currdir()->files[x].m
#define memfilename(x) memfile(x)->filename
#define memfilebuf(x) memfile(x)->buffer
#define memfilecb(x) memfile(x)->cb
#define memfilesize(x) memfile(x)->size

#define memslotused(x) currdir()->records[x].used /* 0 if memory slot unavailable, 1 if available */
#define memslottype(x) currdir()->records[x].type /* 1 for file, 2 for directory */

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
	char *fmt_modification_date;
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
	memrecord_t *records;
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