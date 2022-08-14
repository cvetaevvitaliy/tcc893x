TOP_DIR = ../..
include $(TOP_DIR)/Makefile.common

LIB  = libhpd.a
OBJS = libhpd.o

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f $(LIB) *.o
