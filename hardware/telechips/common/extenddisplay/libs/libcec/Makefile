TOP_DIR = ../..
include $(TOP_DIR)/Makefile.common

LIB  = libcec.a
OBJS = libcec.o

CFLAGS += -I../..

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f $(LIB) *.o
