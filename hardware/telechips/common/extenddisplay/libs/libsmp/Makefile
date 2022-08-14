TOP_DIR = ../..
include $(TOP_DIR)/Makefile.common

LIB  = libsmp.a
OBJS = hdmi_sha1.o \
       smp_add.o   \
       smp_div.o   \
       smp_err.o   \
       smp_exp.o   \
       smp_mod.o   \
       smp_mont.o  \
       smp_mul.o   \
       smp_shift.o \
       smp_sqr.o   \
       smp_util.o  \
       arm/smp_asm.o

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $@ $(OBJS)
	$(RANLIB) $@

arm/smp_asm.o:
#	arm-linux-as -mno-fpu arm/smp_asm.s -o arm/smp_asm.o
	$(AS) -mno-fpu arm/smp_asm.s -o arm/smp_asm.o

clean:
	rm -f $(LIB) *.o arm/*.o
