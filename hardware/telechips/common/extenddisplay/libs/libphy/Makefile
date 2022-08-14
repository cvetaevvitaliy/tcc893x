TOP_DIR = ../..
include $(TOP_DIR)/Makefile.common

LIB  = libphy.a ../libiic/libiic.a
OBJS = libphy.o

CFLAGS += -I../ -I../.. -I../../modules/

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f $(LIB) *.o
