/****************************************************************************
 * platform/tcc893x/tcc_asm.S
 * Copyright (C) 2014 Telechips Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions andlimitations under the License.
 ****************************************************************************/

#include <asm.h>
#include <arch/ops.h>
#include <arch/defines.h>
#include <platform/sram_map.h>

#undef ENTRY
#undef ENDPROC
#define ENTRY(x)    FUNCTION(x)
#define ENDPROC(x)

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

