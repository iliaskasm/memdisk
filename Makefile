CC			= gcc
CFLAGS		= -lpthread -g
MEMD_DEPS 	= sharedmem.c memutils.c
MEMC_DEPS 	= sharedmem.c
INSTALL_DIR = $(shell if [ -z `cat .installdir` ]; then echo $$HOME/memdisk; else echo `cat .installdir`; fi)

all: before memd mem

clean: 
	rm -rf $(INSTALL_DIR)/memd $(INSTALL_DIR)/mem

before:
	mkdir -p $(INSTALL_DIR)
	
memd: 
	$(CC) -o $(INSTALL_DIR)/memd $(CFLAGS) memdisk.c $(MEMD_DEPS)

mem: 
	$(CC) -o $(INSTALL_DIR)/mem $(CFLAGS) memclient.c $(MEMC_DEPS)

