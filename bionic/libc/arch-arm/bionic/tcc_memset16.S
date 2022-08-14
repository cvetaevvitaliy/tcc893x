#include <machine/asm.h>

/*
 * void tcc_memset16(void *dst, int c, int n)
 */

ENTRY(tcc_memset16)
	.save {r0, r4-r7, lr}
	stmfd	sp!, {r0, r4-r7, lr}

	/* check odd address */
	tst	r0, #1
	ldmnefd	sp!, {r0,r4-r7, lr}
	bxne	lr

	/* check count */
	movs	r3, r2
	ldmeqfd	sp!, {r0,r4-r7, lr}
	bxeq	lr

	/* align word */
	tst	r0, #2
	strneh	r1, [r0], #2
	subne	r2, r2, #1
	movs	r3, r2
	ldmeqfd	sp!, {r0,r4-r7,lr}
	bxeq	lr

	/* splat r1 */
	orr	r1, r1, lsl #16
	mov	r12, r1
	mov	lr, r1
	mov	r4, r1
	mov	r5, r1
	mov	r6, r1
	mov	r7, r1

	/* align cache-line */
	rsb	r3, r0, #0
	ands	r3, r3, #0x1C
	beq	3f
	mov	r3, r3, lsr #1
	cmp	r3, r2
	andhi	r3, r2, #0x0E
	sub	r2, r2, r3

	movs	r3, r3, lsl #29
	stmcsia	r0!, {r1, lr}
	stmcsia	r0!, {r1, lr}
	stmmiia	r0!, {r1, lr}
	movs	r3, r3, lsl #2
	strcs	r1, [r0], #4

3:	subs	r2, r2, #16
	mov	r3, r1
	bmi	2f
1:	subs	r2, r2, #16
	stmia	r0!, {r1,r3,r4,r5,r6,r7,r12,lr}
	bhs	1b
2:	add	r2, r2, #16
	movs	r2, r2, lsl #29
	stmcsia	r0!, {r1,r3,r12,lr}
	stmmiia	r0!, {r1,lr}
	movs	r2,r2, lsl #2
	strcs	r1, [r0], #4
	strmih	r1, [r0]

	ldmfd	sp!, {r0, r4-r7, lr}
	bx	lr
END(tcc_memset16)
