#include <machine/asm.h>

/*
 * void endian_swap_64(char *dst, char *src, int size)
 */

ENTRY(endian_swap_64)
// 64 bytes aligned.
//R0 : dst address
//R1 : src address
//R2 : count

   STMFD  sp!,{r2-r3,lr}
   MOV     r3,r1
   MOV     lr,r0
loopB2LEW64:
   VLDM r3!, {d0-d7}
   VREV64.8 Q0, Q0
   VREV64.8 Q1, Q1
   VREV64.8 Q2, Q2
   VREV64.8 Q3, Q3
   SUBS r2, r2, #0x40
   VSTM lr!, {d0-d7}
   BGE loopB2LEW64
   LDMFD  sp!,{r2-r3,pc}

END(endian_swap_64)

