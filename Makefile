CC			= gcc
CFLAGS		= -lpthread -g
MEMD_DEPS 	= sharedmem.c memutils.c
MEMC_DEPS 	= sharedmem.c

all: memd memc

clean: 
	rm -rf memd memc

memd: 
	$(CC) -o memd $(CFLAGS) memdisk.c $(MEMD_DEPS)

memc: 
	$(CC) -o memc $(CFLAGS) memclient.c $(MEMC_DEPS)

