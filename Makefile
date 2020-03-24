CC			= gcc
CFLAGS		= -lpthread -g
MEMD_DEPS 	= sharedmem.c memutils.c
MEMC_DEPS 	= sharedmem.c
INSTALL_DIR = $(shell if [ -z `cat .installdir` ]; then echo $$HOME/memdisk; else echo `cat .installdir`; fi)

all: memd mem

clean: 
	rm -rf $(INSTALL_DIR)/bin/memd $(INSTALL_DIR)/mem

memd: 
	$(CC) -o $(INSTALL_DIR)/bin/memd $(CFLAGS) memdisk.c $(MEMD_DEPS)

mem: 
	$(CC) -o $(INSTALL_DIR)/bin/mem $(CFLAGS) memclient.c $(MEMC_DEPS)

