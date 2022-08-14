TOP_DIR = ../..
include $(TOP_DIR)/Makefile.common

LIB  = libedid.a
OBJS = libedid.o edidparse.o

CFLAGS += -I../ -I../..

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f $(LIB) *.o
