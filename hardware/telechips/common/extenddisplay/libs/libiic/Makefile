TOP_DIR = ../..
include $(TOP_DIR)/Makefile.common

LIB  = libiic.a
OBJS = libiic.o

CFLAGS += -I../

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f $(LIB) *.o
