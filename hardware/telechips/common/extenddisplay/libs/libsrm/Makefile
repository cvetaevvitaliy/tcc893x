TOP_DIR = ../..
include $(TOP_DIR)/Makefile.common

LIB = libsrm.a
OBJS = libsrm.o

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f $(LIB) *.o
