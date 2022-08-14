/****************************************************************************
 * arch/arm/mach-tcc893x/tcc_asm.S
 * Copyright (C) 2014 Telechips Inc.
 *
 * This program is free software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
 ****************************************************************************/

#include <linux/linkage.h>
#include <mach/hardware.h>
#include <mach/sram_map.h>
#ifdef CONFIG_ARM_TRUSTZONE
#include <mach/smc.h>
#endif


/*
  CPSR Mode Bit Definition
*/
#define Mode_USR            0x10
#define Mode_FIQ            0x11
#define Mode_IRQ            0x12
#define Mode_SVC            0x13
#define Mode_ABT            0x17
#define Mode_UND            0x1B
#define Mode_SYS            0x1F
#define Mode_MASK           0x1F
#define NOINT               0xC0
#define I_Bit               0x80
#define F_Bit               0x40

/*
  TRUSTZONE TZASC REGISTERS
*/
#define TZASC_DMEM_R0_ATR   0x60300108
#define TZASC_PERI_R0_ATR   0x60310108
#define TZASC_IMEM_R0_ATR   0x60320108

.text

ENTRY(IO_ARM_SetCPSR)
    msr     cpsr_cf, r0
    bx      lr                  @mov    pc, lr
ENDPROC(IO_ARM_SetCPSR)


ENTRY(IO_ARM_GetCPSR)
    mrs     r0, cpsr
    bx      lr                  @mov    pc, lr
ENDPROC(IO_ARM_GetCPSR)


ENTRY(IO_ARM_ChangeStackSRAM)
    mov     r0, r13
    ldr     r13, =SRAM_STACK_ADDR
    mov     pc, lr
ENDPROC(IO_ARM_ChangeStackSRAM)


ENTRY(IO_ARM_RestoreStackSRAM)
    mov     r13, r0
    mov     pc, lr
ENDPROC(IO_ARM_RestoreStackSRAM)

#ifdef CONFIG_SHUTDOWN_MODE
/*
  r0 : shut-down func phy address
  r1 : mmu & cpu_reg backup area vir address
  r2 : restore func vir address
*/
ENTRY(save_cpu_reg)
    @ shut-down func address
    mov     r12, r0

    @ save svc_reg (r4-r12,lr)
    stmdb   sp!, {r4-r12}
    stmdb   sp!, {lr}

    @ mark shut down mode
    ldr     r3, =0x18C818C8
    str     r3, [r1, #0x00]

    @ save restore func address
    str     r2, [r1, #0x04]
    mov     r3, r1

    @ save SCTLR
    mrc     p15, 0, r2, c1, c0, 0
    str     r2, [r3, #0x08]

    @ save TTBR0
    mrc     p15, 0, r2, c2, c0, 0
    str     r2, [r3, #0x0C]

    @ save TTBR1
    mrc     p15, 0, r2, c2, c0, 1
    str     r2, [r3, #0x10]

    @ save TTBCR
    mrc     p15, 0, r2, c2, c0, 2
    str     r2, [r3, #0x14]

    @ save DACR
    mrc     p15, 0, r2, c3, c0, 0
    str     r2, [r3, #0x18]

    @ save DFSR
    mrc     p15, 0, r2, c5, c0, 0
    str     r2, [r3, #0x1C]

    @ save DFAR
    mrc     p15, 0, r2, c6, c0, 0
    str     r2, [r3, #0x20]

    @ save IFSR
    mrc     p15, 0, r2, c5, c0, 1
    str     r2, [r3, #0x24]

    @ save IFAR
    mrc     p15, 0, r2, c6, c0, 2
    str     r2, [r3, #0x28]

    @ save DAFSR
    mrc     p15, 0, r2, c5, c1, 0
    str     r2, [r3, #0x2C]

    @ save IAFSR
    mrc     p15, 0, r2, c5, c1, 1
    str     r2, [r3, #0x30]

    @ save PMRRR
    mrc     p15, 0, r2, c10, c2, 0
    str     r2, [r3, #0x34]

    @ save NMRRR
    mrc     p15, 0, r2, c10, c2, 1
    str     r2, [r3, #0x38]

#ifndef CONFIG_TRUSTED_FOUNDATIONS
    mrc     p15, 0, r2, c15, c0, 0      @ read power control register
    str     r2, [r3, #0xA0]
    mrc     p15, 0, r2, c15, c0, 1      @ diag
    str     r2, [r3, #0xA4]
#endif
    mrc     p15, 0, r2, c13, c0, 0      @ FCSE/PID
    str     r2, [r3, #0xA8]
    mrc     p15, 0, r2, c13, c0, 3      @ User r/o thread ID
    str     r2, [r3, #0xAC]
    mrc     p15, 0, r2, c1, c0, 1       @ Auxiliary control register
    str     r2, [r3, #0xB0]
    mrc     p15, 0, r2, c1, c0, 2       @ Co-processor access control
    str     r2, [r3, #0xB4]

    @ save svc_reg (spsr)
    str     sp, [r3, #0x3C]
    mrs     r2, spsr
    str     r2, [r3, #0x40]

    add     r3, r3, #0x00000044

    @ save fiq_reg
    mov     r1, #Mode_FIQ | NOINT       @ Enter FIQ mode, no interrupts
    msr     cpsr, r1
    mrs     r2, spsr                    @ Status Register
    stmia   r3!, {r2, r8-r12, sp, lr}   @ Store FIQ mode registers [SleepState_FIQ_SPSR~SleepState_FIQ_LR]

    @ save abort_reg
    mov     r1, #Mode_ABT | NOINT       @ Enter ABT mode, no interrupts
    msr     cpsr, r1
    mrs     r0, spsr                    @ Status Register
    stmia   r3!, {r0, sp, lr}           @ Store ABT mode Registers [SleepState_ABT_SPSR~SleepState_ABT_LR]

    @ save irq_reg
    mov     r1, #Mode_IRQ | NOINT       @ Enter IRQ mode, no interrupts
    msr     cpsr, r1
    mrs     r0, spsr                    @ Status Register
    stmia   r3!, {r0, sp, lr}           @ Store the IRQ Mode Registers [SleepState_IRQ_SPSR~SleepState_IRQ_LR]
    
    @ save undef_reg
    mov     r1, #Mode_UND | NOINT       @ Enter UND mode, no interrupts
    msr     cpsr, r1
    mrs     r0, spsr                    @ Status Register
    stmia   r3!, {r0, sp, lr}           @ Store the UND mode Registers [SleepState_UND_SPSR~SleepState_UND_LR]

    @ save usr(sys)_reg
    mov     r1, #Mode_SYS | NOINT       @ Enter SYS mode, no interrupts
    msr     cpsr, r1
    stmia   r3!, {sp, lr}               @ Store the SYS mode Registers [SleepState_SYS_SP, SleepState_SYS_LR]

    @ Return to SVC mode
    mov     r1, #Mode_SVC | NOINT       @ Back to SVC mode, no interrupts
    msr     cpsr, r1
    
    ldr     r2, [sp]
    str     r2, [r3], #4    

    @ invalidate entire tlb & btb & prefetch buffer.
    mov     r0, #0x0
    dsb     sy
    mcr     p15, 0, r0, c8, c7, 0       @ p15,0,r3,c8,c7,0 (itlb+dtlb flush all)
    mcr     p15, 0, r0, c7, c5, 6       @ p15,0,r3,c7,c5,6 (invalidate btb)
    mcr     p15, 0, r0, c7, c5, 4       @ flush prefetch buffer
    dsb     sy
    isb     sy

    @ clean & invalidate cache(L1)
    bl      v7_flush_kern_cache_all

    @ Data synchronization barrier operation
    mov     r0, #0
    mcr     p15, 0, r0, c7, c10, 4

    @ set stack to SRAM
    ldr     sp, =SRAM_STACK_ADDR

    @ Jump to shutdown()
    mov     pc, r12
ENDPROC(save_cpu_reg)


ENTRY(SRAM_Boot)
//    ldr     pc, =0xF0000004             @ Jump to remap address for MMU control.

//wakeup_test_loop:
//    b       wakeup_test_loop

#ifdef CONFIG_ARM_TRUSTZONE
    ldr     r0, =0x60330040
    ldr     r1, [r0]
    bic     r1, r1, #0x7
    str     r1, [r0]

    #if 0    // for j-tag debugg int secure & non-secure world
    mrc     p15, 0, r2, c1, c1, 1
    orr     r2, r2, #0x3
    mcr     p15, 0, r2, c1, c1, 1
    #endif
#endif

    @ check cpuid
    mrc     p15, 0, r0, c0, c0, 5
    and     r0, r0, #0x3
    mov     r1, #0
    cmp     r0, r1
    ldrne   pc, =0x30000000

    @ TZASC set all-regions to non-secure --> uskim @ 2012.08.22
    ldr     r0, =TZASC_DMEM_R0_ATR
    mov     r1, #0xf0000000
    str     r1, [r0]
    add     r0,r0,#0x10000
    str     r1, [r0]
    add     r0,r0,#0x10000
    str     r1, [r0]

    @ Mask IRQ/FIQ
    mrs     r0, cpsr
    orr     r0, r0, #0xC0
    msr     cpsr_cf, r0

    @ Set sp(stack point)
    ldr     r0, =SRAM_STACK_ADDR
    bic     r0, r0, #(0xF<<28)
    mov     r13, r0

    @ Jump to wakeup()
    ldr     r0, =WAKEUP_FUNC_ADDR
    bic     r0, r0, #(0xF<<28)
    mov     lr, pc
    mov     pc, r0

    @ Jump to sdram_init()
    ldr     r0, =SDRAM_INIT_FUNC_ADDR
    bic     r0, r0, #(0xF<<28)
    mov     lr, pc
    mov     pc, r0

    @Jump to remap address for MMU control.
    mov     r12, pc
    orr     pc, r12, #0xF0000000

#ifdef CONFIG_ARM_TRUSTZONE
    mov     lr, pc
    ldr     pc, =0x9FF00000
#endif

    @ Check whether Shutdown is enabled or not.
    @ cpu_reg/mmu data backup address
    ldr     r12, =CPU_DATA_REPOSITORY_ADDR
    ldr     r1, [r12]
    ldr     r2, =0x18C818C8
    cmp     r1, r2
    ldr     r1, =0x0
    str     r1, [r12]

wakeup_err_loop:
    bne     wakeup_err_loop

    @ DCache Invalidate..
    dmb                                 @ ensure ordering with previous memory accesses
    mrc     p15, 1, r0, c0, c0, 1       @ read clidr
    ands    r3, r0, #0x7000000          @ extract loc from clidr
    mov     r3, r3, lsr #23             @ left align loc bit field
    beq     finished                    @ if loc is 0, then no need to clean
    mov     r10, #0                     @ start clean at cache level 0
loop1:
    add     r2, r10, r10, lsr #1        @ work out 3x current cache level
    mov     r1, r0, lsr r2              @ extract cache type bits from clidr
    and     r1, r1, #7                  @ mask of the bits for current cache only
    cmp     r1, #2                      @ see what cache we have at this level
    blt     skip                        @ skip if no cache, or just i-cache
    mcr     p15, 2, r10, c0, c0, 0      @ select current cache level in cssr
    isb                                 @ isb to sych the new cssr&csidr
    mrc     p15, 1, r1, c0, c0, 0       @ read the new csidr
    and     r2, r1, #7                  @ extract the length of the cache lines
    add     r2, r2, #4                  @ add 4 (line length offset)
    ldr     r4, =0x3ff
    ands    r4, r4, r1, lsr #3          @ find maximum number on the way size
    clz     r5, r4                      @ find bit position of way size increment
    ldr     r7, =0x7fff
    ands    r7, r7, r1, lsr #13         @ extract max number of the index size
loop2:
    mov     r9, r4                      @ create working copy of max way size
loop3:
    orr     r11, r10, r9, lsl r5        @ factor way and cache number into r11
    orr     r11, r11, r7, lsl r2        @ factor index number into r11
    mcr     p15, 0, r11, c7, c6, 2      @ invalidate by set/way
    subs    r9, r9, #1                  @ decrement the way
    bge     loop3
    subs    r7, r7, #1                  @ decrement the index
    bge     loop2
skip:
    add     r10, r10, #2                @ increment cache number
    cmp     r3, r10
    bgt     loop1
finished:
    mov     r10, #0                     @ swith back to cache level 0
    mcr     p15, 2, r10, c0, c0, 0      @ select current cache level in cssr
    dsb
    isb

    @ ICache Invalidate..
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 0       @ I+BTB cache invalidate

    @ disable mmu & cache
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #0x0005             @ Disable MMU and Data Cache
    mcr     p15, 0, r0, c1, c0, 0

    @ invalidate tlb
    mov     r0, #0
    mcr     p15, 0, r0, c8, c7, 0

    @ set reserved context ID
    mcr     p15, 0, r0, c13, c0, 1      @ set reserved context ID

#ifndef CONFIG_TRUSTED_FOUNDATIONS
    @ Power control register
    ldr     r0, [r12, #0xA0]
#ifdef CONFIG_ARM_TRUSTZONE
    mov     r1, r0
    ldr     r0, =SMC_CMD_CORTEXA9_POWER_CTRL
    smc     0
#else
    mcr     p15, 0, r10, c15, c0, 0     @ write power control register
#endif

    @ Diagnostic
    ldr     r0, [r12, #0xA4]
#ifdef CONFIG_ARM_TRUSTZONE
    mov     r1, r0
    ldr     r0, =SMC_CMD_CORTEXA9_DIAG
    smc     0
#else
    mcr     p15, 0, r0, c15, c0, 1      @ diag
#endif
#endif

    @ FCSE/PID
    ldr     r0, [r12, #0xA8]
    mcr     p15, 0, r0, c13, c0, 0      @ FCSE/PID

    @ User r/o thread ID
    ldr     r0, [r12, #0xAC]
    mcr     p15, 0, r0, c13, c0, 3      @ User r/o thread ID

    @ Auxiliary control register
    ldr     r0, [r12, #0xB0]
    mrc     p15, 0, r10, c1, c0, 1      @ Read Auxiliary control register
    teq     r10, r0                     @ Is it already set?
#if (1)
    beq     skip_actlr_setting
#ifdef      CONFIG_ARM_TRUSTZONE
    mov     r1, r0
    ldr     r0, =SMC_CMD_ARM_ACTLR
    smc     0
#else
    mcr     p15, 0, r0, c1, c0, 1
#endif
skip_actlr_setting:
#else
    mcrne   p15, 0, r0, c1, c0, 1       @ No, so write it
#endif

    @ Co-processor access control
    ldr     r0, [r12, #0xB4]
    mcr     p15, 0, r0, c1, c0, 2       @ Co-processor access control

    @ restore DACR
    ldr     r0, [r12, #0x18]
    mcr     p15, 0, r0, c3, c0, 0

    @ restore TTBCR
    ldr     r0, [r12, #0x14]
    mcr     p15, 0, r0, c2, c0, 2

    @ restore TTBR1
    ldr     r0, [r12, #0x10]
    mcr     p15, 0, r0, c2, c0, 1

    @ restore TTBR0
    ldr     r0, [r12, #0x0C]
    mcr     p15, 0, r0, c2, c0, 0

    @ DFSR
    ldr     r0, [r12, #0x1C]
    mcr     p15, 0, r0, c5, c0, 0

    @ DFAR
    ldr     r0, [r12, #0x20]
    mcr     p15, 0, r0, c6, c0, 0

    @ IFSR
    ldr     r0, [r12, #0x24]
    mcr     p15, 0, r0, c5, c0, 1

    @ IFAR
    ldr     r0, [r12, #0x28]
    mcr     p15, 0, r0, c6, c0, 2

    @ DAFSR
    ldr     r0, [r12, #0x2C]
    mcr     p15, 0, r0, c5, c1, 0

    @ IAFSR
    ldr     r0, [r12, #0x30]
    mcr     p15, 0, r0, c5, c1, 1

    @ PMRRR
    ldr     r0, [r12, #0x34]
    mcr     p15, 0, r0, c10, c2, 0

    @ NMRRR
    ldr     r0, [r12, #0x38]
    mcr     p15, 0, r0, c10, c2, 1

    @ restore SCTLR
    ldr     r0, [r12, #0x08]            @ System Control Register
    mcr     p15, 0, r0, c1, c0, 0       @ mmu & cache on
    nop

    @ REMAP 0xF0000000 to DRAM
    ldr     r0, =0xF4810010
    ldr     r3, [r0]
    bic     r3, r3, #(1<<31)
    str     r3, [r0]

    @ Jump to resore_cpu_reg()
    ldr     r1, [r12, #0x04]
    mov     pc, r1
ENDPROC(SRAM_Boot)


ENTRY(resore_cpu_reg)

    @ cpu_reg/mmu data backup address
    ldr     r3, =CPU_DATA_REPOSITORY_ADDR

    @ restore fiq_reg
    mov     r1, #Mode_FIQ | NOINT
    msr     cpsr, r1
    ldr     r0, [r3, #0x44]
    msr     spsr, r0
    ldr     r8, [r3, #0x48]
    ldr     r9, [r3, #0x4C]
    ldr     r10,[r3, #0x50]
    ldr     r11,[r3, #0x54]
    ldr     r12,[r3, #0x58]
    ldr     sp, [r3, #0x5C]
    ldr     lr, [r3, #0x60]

    @ restore abort_reg
    mov     r1, #Mode_ABT | I_Bit
    msr     cpsr, r1
    ldr     r0, [r3, #0x64]
    msr     spsr, r0
    ldr     sp, [r3, #0x68]
    ldr     lr, [r3, #0x6C]

    @ restore irq_reg
    mov     r1, #Mode_IRQ | I_Bit
    msr     cpsr, r1
    ldr     r0, [r3, #0x70]
    msr     spsr, r0
    ldr     sp, [r3, #0x74]
    ldr     lr, [r3, #0x78]

    @ restore undef_reg
    mov     r1, #Mode_UND | I_Bit
    msr     cpsr, r1
    ldr     r0, [r3, #0x7C]
    msr     spsr, r0
    ldr     sp, [r3, #0x80]
    ldr     lr, [r3, #0x84]

    @ restore usr(sys)_reg
    mov     r1, #Mode_SYS | I_Bit
    msr     cpsr, r1
    ldr     sp, [r3, #0x88]
    ldr     lr, [r3, #0x8C]

    @ restore svc_reg
    mov     r1, #Mode_SVC | I_Bit
    msr     cpsr, r1
    ldr     r0, [r3, #0x40]
    msr     spsr, r0
    ldr     sp, [r3, #0x3C]
    ldr     lr, [sp], #4
    ldmia   sp!, {r4-r12}

    mov     pc, lr
ENDPROC(resore_cpu_reg)
#endif /* CONFIG_SHUTDOWN_MODE */

/*
 * tcc_cpu_wfi
 *
 * puts current CPU in wfi
 * CPU core clock-gates itself during wfi
 *
 * corrupts r0-r3
 * must be called with MMU on
 */
ENTRY(tcc_cpu_wfi)
    dsb
    wfi
    mov     pc, lr
ENDPROC(tcc_cpu_wfi)
