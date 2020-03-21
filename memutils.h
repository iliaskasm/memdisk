/*
 * Memdisk utilities header file
 *
 * Ilias K. Kasmeridis, 2018
 */

#ifndef __MEMUTILS_H__
#define __MEMUTILS_H__

#include "memdisk.h"

char *tmpdir(char *s);
int findf(memfolder_t *folder, char *filename);
int isbin(memfolder_t *folder, int x);
off_t fsize(const char *filename);

#endif