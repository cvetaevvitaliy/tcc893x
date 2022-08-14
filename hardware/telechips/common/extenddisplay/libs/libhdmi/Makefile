TOP_DIR = ../..
include $(TOP_DIR)/Makefile.common

LIB  = libhdmi.a ../libphy/libphy.a
OBJS = libhdmi.o

CFLAGS += -I../ -I../.. -I../../modules/

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f $(LIB) *.o
