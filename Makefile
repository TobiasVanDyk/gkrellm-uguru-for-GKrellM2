
PREFIX ?= /usr/local
GTK_CONFIG = pkg-config gtk+-2.0
PLUGIN_DIR ?= $(PREFIX)/lib/gkrellm2/plugins
GKRELLM_INCLUDE = -I$(PREFIX)/include
GTK_CFLAGS = `$(GTK_CONFIG) --cflags` 
GTK_LIB = `$(GTK_CONFIG) --libs`
FLAGS = -Wall -fPIC $(GTK_CFLAGS) $(GKRELLM_INCLUDE)
CFLAGS ?= -O -g
CFLAGS += $(FLAGS)
LIBS = $(GTK_LIB)
LFLAGS = -shared
CC ?= gcc
INSTALL = install -c
INSTALL_PROGRAM = $(INSTALL) -s
OBJS = ugurup2.o

ugurup1.so: $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o ugurup2.so $(LIBS) $(LFLAGS)

clean:
	rm -f *.o core *.so* *.bak *~

install: 
	$(INSTALL_PROGRAM) ugurup2.so $(PLUGIN_DIR)	

%.c.o: %.c

