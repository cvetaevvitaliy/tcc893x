@ /****************************************************************************
@ Copyright (C) 2013 Telechips Inc.
@ 
@ Licensed under the Apache License, Version 2.0 (the "License");
@ you may not use this file except in compliance with the License.
@ You may obtain a copy of the License at
@
@ http://www.apache.org/licenses/LICENSE-2.0
@
@ Unless required by applicable law or agreed to in writing, software
@ distributed under the License is distributed on an "AS IS" BASIS,
@ WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@ See the License for the specific language governing permissions and
@ limitations under the License.
@ ****************************************************************************/

@ ///////////////////////////////////////////////////////////////////
@  @file smp_asm.s
@  @brief
@  Assembly version of smp_asm.c functions for higher performance
@
@ ///////////////////////////////////////////////////////////////////

@ -------------------------------------------------------------------
@  Function Implementation
@ -------------------------------------------------------------------

@================================================
@ Name : smp_add_words(SMP_WORD *r,
@                      const SMP_WORD *a,
@                      const SMP_WORD *b,
@                      int num)
@ Function : Add words with equal length
@================================================

        .text
        .global smp_add_words;
        .type smp_add_words,%function
        .align 4;

smp_add_words:
    STMFD    sp!, {r4-r6,lr}
    CMP        r3, #0         @ Return if length is zero
    BEQ        add_zero
    MOV        r6, #0         @ Initialize carry
add_loop:
    LDR        r4, [r1], #4   @ Load a[i]
    LDR        r5, [r2], #4   @ Load b[i]
    ADDS    r4, r4, r6     @ a[i]+carry
    MOV        r6, #0
    ADC        r6, r6, #0     @ check carry
    ADDS    r4, r4, r5     @ a[i]+b[i]
    ADC        r6, r6, #0     @ check carry
    STR        r4, [r0], #4   @ Store rp[i]
    SUBS    r3, r3, #1
    BNE        add_loop
    MOV        r0, r6         @ Return last carry
    B        smp_add_end
add_zero:
    MOV        r0, #0
smp_add_end:
    LDMFD    sp!, {r4-r6,pc}


@================================================
@ Name : smp_sub_words(SMP_WORD *r,
@                      const SMP_WORD *a,
@                      const SMP_WORD *b,
@                      int num)
@ Function : Subtract words with equal length
@================================================

        .text
        .global smp_sub_words;
        .type smp_sub_words,%function
        .align 4;

smp_sub_words:
    STMFD    sp!, {r4-r6,lr}
    CMP        r3, #0         @ Return if length is zero
    BEQ        sub_zero
    MOV        r6, #0         @ Initialize carry
sub_loop:
    LDR        r4, [r1], #4   @ Load a[i]
    LDR        r5, [r2], #4   @ Load b[i]
    SUBS    r4, r4, r6     @ a[i]-carry
    MOV        r6, #0
    ADDMI    r6, r6, #1     @ check carry
    SUBS    r4, r4, r5     @ a[i]-b[i]
    ADDMI    r6, r6, #1     @ check carry
    STR        r4, [r0], #4   @ Store rp[i]
    SUBS    r3, r3, #1
    BNE        sub_loop
    MOV        r0, r6         @ Return last carry
    B        smp_sub_end
sub_zero:
    MOV        r0, #0
smp_sub_end:
    LDMFD    sp!, {r4-r6,pc}


@================================================
@ Name : smp_mul_words(SMP_WORD *rp,
@                      const SMP_WORD *ap,
@                      int num,
@                      SMP_WORD w)
@ Function : Multiply SMP_WORD array with word
@================================================

        .text
        .global smp_mul_words;
        .type smp_mul_words,%function
        .align 4;

smp_mul_words:
    STMFD    sp!, {r4-r7,lr}
    CMP        r2, #0         @ Return if length is zero
    BEQ        mul_zero
    MOV        r7, #0         @ Initialize carry
mul_loop:
    LDR        r4, [r1], #4   @ Load ap[i]
    UMULL    r5, r6, r4, r3 @ (r6,r5) := ap[i]*w
    ADDS    r5, r5, r7     @ lsb = lsb+carry
    ADC        r7, r6, #0     @ carry = msb+(carry from above)
    STR        r5, [r0], #4   @ Store rp[i]
    SUBS    r2, r2, #1
    BNE        mul_loop
    MOV        r0, r7         @ Return last carry
    B        smp_mul_end
mul_zero:
    MOV        r0, #0
smp_mul_end:
    LDMFD    sp!, {r4-r7,pc}


@================================================
@ Name : smp_mul_add_words(SMP_WORD *rp,
@                          const SMP_WORD *ap,
@                          int num,
@                          SMP_WORD w)
@ Function : Multiply and accumulate result
@================================================

        .text
        .global smp_mul_add_words;
        .type smp_mul_add_words,%function
        .align 4;

smp_mul_add_words:
    STMFD    sp!, {r4-r8,lr}
    CMP        r2, #0         @ Return if length is zero
    BEQ        mul_add_zero
    MOV        r8, #0         @ Initialize carry
mul_add_loop:
    LDR        r4, [r1], #4   @ Load ap[i]
    LDR        r5, [r0]       @ Load rp[i]
    UMULL    r6, r7, r4, r3 @ (r7,r6) := ap[i]*w
    ADDS    r6, r6, r8     @ lsb = lsb+carry
    ADC        r8, r7, #0     @ carry = msb+(carry from above)
    ADDS    r5, r5, r6     @ rp[i] = rp[i]+lsb
    ADC        r8, r8, #0     @ carry check
    STR        r5, [r0], #4   @ Store rp[i]
    SUBS    r2, r2, #1
    BNE        mul_add_loop
    MOV        r0, r8         @ Return last carry
    B        smp_mul_add_end
mul_add_zero:
    MOV        r0, #0
smp_mul_add_end:
    LDMFD    sp!, {r4-r8,pc}


@================================================
@ Name : smp_sqr_words(SMP_WORD *r,
@                      const SMP_WORD *a,
@                      int num)
@ Function : Square SMP_WORD array
@================================================

        .text
        .global smp_sqr_words;
        .type smp_sqr_words,%function
        .align 4;

smp_sqr_words:
    STMFD    sp!, {r3-r5,lr}
    CMP        r2, #0         @ Return if length is zero
    BEQ        smp_sqr_end
sqr_loop:
    LDR        r3, [r1], #4   @ Load ap[i]
    UMULL    r4, r5, r3, r3 @ (r5,r4) := ap[i]^2
    STR        r4, [r0], #4   @ Store LSW rp[i]
    STR        r5, [r0], #4   @ Store MSW rp[i]
    SUBS    r2, r2, #1
    BNE        sqr_loop
smp_sqr_end:
    LDMFD    sp!, {r3-r5,pc}
