#-------------------------------------------------------------#
# Copyright (c) 2009 Telechips Inc.                           #
# ALL RIGHTS RESERVED                                         #
#-------------------------------------------------------------#

%.so:
	@$(ECHO) -e "[\x1b[1;32mSO\x1b[0m] $^ => $@ (Shared)"
	@$(CC) $(DEF_LDFLAGS_SHARED) -o $@ $^

%.a:
	@$(ECHO) -e "[\x1b[1;37mST\x1b[0m] $^ => $@ (Static)"
	@$(AR) $(DEF_ARFLAGS_STATIC) $@ $^

lib%.lo:
	@$(ECHO) -e "[\x1b[1;36mLO\x1b[0m] $^ => $@ (Link object)"
	@$(LD) $(DEF_LDFLAGS_OBJECT) -o $@ $^

ANSI_%.o: %.c $(DEF_DEPEND_COMMON) 
	@$(ECHO) -e "[\x1b[1;34mCC\x1b[0m] $< => $@ (ANSI)"
	@$(CC) $(DEF_CFLAGS_ANSI) -o $@ $<

%.o: %.c $(DEF_DEPEND_COMMON) 
	@$(ECHO) -e "[\x1b[1;34mCC\x1b[0m] $< => $@ (GNUC)"
	@$(CC) $(DEF_CFLAGS_GNUC) -o $@ $<

%.o: %.s
	@$(ECHO) -e "[\x1b[1;34mAS\x1b[0m] $< => $@ (ASSEMBLE)"
	@$(AS) -o $@ $<

# End of make 
