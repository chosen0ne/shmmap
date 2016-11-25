uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
OPTIMIZATION?=-O2

# Default settings
STD=-std=c99
WARN=-Wall -W
OPT=$(OPTIMIZATION)
DEBUG=-g -ggdb

FINAL_CFLAGS=$(STD) $(WARN) $(OPT) $(CFLAGS) $(DEBUG)
FINAL_LDFLAGS=$(LDFLAGS) $(DEBUG)

ifeq ($(uname_S),Linux)
	FINAL_CFLAGS+= -D_GNU_SOURCE
endif

SHMMAP_CC=$(QUIET_CC)$(CC) $(FINAL_CFLAGS)
SHMMAP_AR=$(QUIET_AR)ar crv
SHMMAP_LD=$(QUIET_LINK)$(CC) $(FINAL_LDFLAGS)

CCCOLOR="\033[34m"
LINKCOLOR="\033[34;1m"
SRCCOLOR="\033[33m"
BINCOLOR="\033[37;1m"
MAKECOLOR="\033[32;1m"
ENDCOLOR="\033[0m"

ifndef V
QUIET_CC = @printf '    %b %b\n' $(CCCOLOR)CC$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_AR = @printf '    %b %b\n' $(LINKCOLOR)AR$(ENDCOLOR) $(BINCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_LINK = @printf '    %b %b\n' $(LINKCOLOR)LINK$(ENDCOLOR) $(BINCOLOR)$@$(ENDCOLOR) 1>&2;
endif

SHMMAP_LIB=libshmmap.a
SHMMAP_TEST_BIN=shmmap_test
SHMMAP_OBJ=m_pool.o shm_map.o

all: $(SHMMAP_LIB) $(SHMMAP_OBJ) $(SHMMAP_TEST_BIN)

$(SHMMAP_LIB): $(SHMMAP_OBJ)
	$(SHMMAP_AR) $(SHMMAP_LIB) $(SHMMAP_OBJ) 1>&2

$(SHMMAP_TEST_BIN): $(SHMMAP_LIB) shmmap_test.o
	$(SHMMAP_LD) -o $@ $^ $(SHMMAP_LIB)

%.o: %.c
	$(SHMMAP_CC) -c $<

clean:
	rm -rf $(SHMMAP_LIB) $(SHMMAP_TEST_BIN) *.o *.dat

.PHONY: clean

noopt:
	$(MAKE) OPTIMIZATION="-O0"

