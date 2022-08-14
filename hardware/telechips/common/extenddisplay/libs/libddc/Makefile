TOP_DIR = ../..
include $(TOP_DIR)/Makefile.common

LIB  = libddc.a
OBJS = libddc.o

CFLAGS += -I../ -I../..

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f $(LIB) *.o
