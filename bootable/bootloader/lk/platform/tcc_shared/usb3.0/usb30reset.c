/*
 * Copyright (c) 2012 Telechips, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "usb3.0/dev_hsbuscfg.h"

#define HwHSB_CFG ((volatile HSBUSCFG *)(0x71EA0000))

// Register Read/Write command
#define REG_WR32(a,b)	    (*((volatile int *)(a)) = b)
#define REG_RD32(a,b)		(b=*((volatile int *)(a)))

void reset_phy_cfg_register (void)
{
    // Write Reset values of All PHY & LINK CFG Registers
    // USB30PHY_CFG0 : 0x9548_4068
    // USB30PHY_CFG1 : 0x0000_041C
    // USB30PHY_CFG2 : 0x919E_14C8
    // USB30PHY_CFG3 : 0x4AB0_5D00
    // USB30PHY_CFG4 : 0x0000_0000
    // USB30PHY_CFG5 : 0x0010_8001
    HwHSB_CFG->uUSB30PHY_CFG0.nReg = 0xB5484068;
    HwHSB_CFG->uUSB30PHY_CFG1.nReg = 0x0000041C;
    HwHSB_CFG->uUSB30PHY_CFG2.nReg = 0x919E14C8;
    HwHSB_CFG->uUSB30PHY_CFG3.nReg = 0x4AB05D00;
    HwHSB_CFG->uUSB30PHY_CFG4.nReg = 0x00000000;
    HwHSB_CFG->uUSB30PHY_CFG5.nReg = 0x00108001;
    HwHSB_CFG->uUSB30LINK_CFG.nReg = 0x80420013;
}


void reset_usb30_poweron(void)
{
    unsigned int tmp;

    // Resets
    //HwHSB_CFG->uUSB30LINK_CFG.bReg.vaux_reset_n : Reset PHY, Controller
    //HwHSB_CFG->uUSB30LINK_CFG.bReg.vcc_reset_n : Reset AXI I/F of Controller 
    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst : Reset HS Circuit of PHY
    //HwUSB30->sGlobal.uGUSB3PIPECTL.bReg.PHYSoftRst : Reset SS Circuit of PHY
    //HwHSB_CFG->uUSB30PHY_CFG1.bReg.PORTRESET : UTMI Reset
    //HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_ssp : Turn off SS Circuit of PHY
    //HwHSB_CFG->uUSB30LINK_CFG.bReg.USB20ONLY : Use with test_powerdown_ssp
    //HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_hsp : Turn off HS Circuit of PHY

    // Spare resets
    //HwHSB_CFG->uUSB30PHY_CFG1.bReg.phy_reset : Initialize 0 (deassert) when power-on reset, and don't use this. After por, use GUSB2PHYCFG.bReg.PHYSoftRst instead
    //HwHSB_CFG->uUSB30PHY_CFG1.bReg.pipe_reset_n : Initialize 1 (deassert) when power-on reset, and don't use this. Afer por, use GUSB3PIPECTL.bReg.PHYSoftRst instead

    
    // Reset PHY CFG registers
    reset_phy_cfg_register();

    // Assert global reset
    HwHSB_CFG->uUSB30LINK_CFG.bReg.vaux_reset_n = 0; // Reset USB30 PHY, USB30 Controller
    HwHSB_CFG->uUSB30LINK_CFG.bReg.vcc_reset_n = 0; // Reset USB30 AXI Bus I/F
    HwHSB_CFG->uUSB30LINK_CFG.bReg.vcc_reset_n = 1; // Release USB30 AXI Bus I/F reset

    // Release partial resets but PHY and controller is in reset because of vaux_reset_n = 0
    HwHSB_CFG->uUSB30PHY_CFG1.bReg.phy_reset = 0; // Release HS PHY but still in reset via vaux_reset_n = 0. After Power-on reset, don't use this register. use GUSB2PHYCFG.PHYSoftRst.
    HwHSB_CFG->uUSB30PHY_CFG1.bReg.PORTRESET = 0; // Release UTMI I/F but still in reset via vaux_reset_n = 0
    HwHSB_CFG->uUSB30PHY_CFG1.bReg.pipe_reset_n = 1; // Release pipe_reset_n but still in reset via vaux_reset_n = 0

    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 0;
    REG_RD32(0x7100C200,tmp);
    tmp &= ~(0x1<<31);
    REG_WR32(0x7100C200,tmp);
    //HwUSB30->sGlobal.uGUSB3PIPECTL.bReg.PHYSoftRst = 0;
    REG_RD32(0x7100C2C0,tmp);
    tmp &= ~(0x1<<31);
    REG_WR32(0x7100C2C0,tmp);

    // Wait T1 (>10us)
    wait_phy_reset_timing(300*2);

    HwHSB_CFG->uUSB30LINK_CFG.bReg.vaux_reset_n = 1; // Release Global reset

    // Wait T3-T1 (>165us)
    wait_phy_reset_timing(4800*2);
}

void set_mode_usb20_only (void)
{
    // USB30 IP acts as only USB20 (HS/FS)
    HwHSB_CFG->uUSB30LINK_CFG.bReg.USB20ONLY = 1;
}

void set_mode_usb30 (void)
{
    // USB30 IP acts USB30 (SS/HS/FS)
    HwHSB_CFG->uUSB30LINK_CFG.bReg.USB20ONLY = 0;
}

void powerdown_ss (void)
{
    unsigned int tmp;

    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 1; // Assert Global PHY reset
    REG_RD32(0x7100C200,tmp);
    tmp |= (0x1<<31);
    REG_WR32(0x7100C200,tmp);

    HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_ssp = 1; // turn off SS circuit

    // Wait T1 (>10us)
    wait_phy_reset_timing(300*2);

    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 0; // De-Assert Global PHY reset
    REG_RD32(0x7100C200,tmp);
    tmp &= ~(0x1<<31);
    REG_WR32(0x7100C200,tmp);

    // Wait T3-T1 (>165us)
    wait_phy_reset_timing(4800*2);
}

void poweron_ss (void)
{
    unsigned int tmp;

    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 1; // Assert Global PHY reset
    REG_RD32(0x7100C200,tmp);
    tmp |= (0x1<<31);
    REG_WR32(0x7100C200,tmp);

    HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_ssp = 0; // turn on SS circuit
    //HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_hsp = 0; // turn on HS circuit

    // Wait T1 (>10us)
    wait_phy_reset_timing(300*2);

    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 0; // De-Assert Global PHY reset
    REG_RD32(0x7100C200,tmp);
    tmp &= ~(0x1<<31);
    REG_WR32(0x7100C200,tmp);

    // Wait T3-T1 (>165us)
    wait_phy_reset_timing(4800*2);
}

void powerdown_hs (void)
{
    unsigned int tmp;

    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 1; // Assert Global PHY reset
    REG_RD32(0x7100C200,tmp);
    tmp |= (0x1<<31);
    REG_WR32(0x7100C200,tmp);

    HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_hsp = 1; // turn off HS circuit
    //HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_ssp = 0; // turn on SS circuit

    // Wait T1 (>10us)
    wait_phy_reset_timing(300*2);

    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 0; // De-Assert Global PHY reset
    REG_RD32(0x7100C200,tmp);
    tmp &= ~(0x1<<31);
    REG_WR32(0x7100C200,tmp);

    // Wait T3-T1 (>165us)
    wait_phy_reset_timing(4800*2);
}

void poweron_hs (void)
{
    unsigned int tmp;

    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 1; // Assert Global PHY reset
    REG_RD32(0x7100C200,tmp);
    tmp |= (0x1<<31);
    REG_WR32(0x7100C200,tmp);

    HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_hsp = 0; // turn on HS circuit
    //HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_ssp = 0; // turn on SS circuit

    // Wait T1 (>10us)
    wait_phy_reset_timing(300*2);

    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 0; // De-Assert Global PHY reset
    REG_RD32(0x7100C200,tmp);
    tmp &= ~(0x1<<31);
    REG_WR32(0x7100C200,tmp);

    // Wait T3-T1 (>165us)
    wait_phy_reset_timing(4800*2);
}

void usb30_software_reset (void)
{
    unsigned int tmp;

    // Assert reset
    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 1;
    REG_RD32(0x7100C200,tmp);
    tmp |= (0x1<<31);
    REG_WR32(0x7100C200,tmp);
    //HwUSB30->sGlobal.uGUSB3PIPECTL.bReg.PHYSoftRst = 1;
    REG_RD32(0x7100C2C0,tmp);
    tmp |= (0x1<<31);
    REG_WR32(0x7100C2C0,tmp);
    //HwUSB30->sGlobal.uGCTL.bReg.CoreSoftReset = 1;
    REG_RD32(0x7100C110,tmp);
    tmp |= (0x1<<11);
    REG_WR32(0x7100C110,tmp);

    // turn on all PHY circuits
    HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_hsp = 0; // turn on HS circuit
    HwHSB_CFG->uUSB30PHY_CFG1.bReg.test_powerdown_ssp = 0; // turn on SS circuit

    // wait T1 (>10us)
    wait_phy_reset_timing(300*2);

    // De-assert reset
    //HwUSB30->sGlobal.uGUSB2PHYCFG.bReg.PHYSoftRst = 0;
    REG_RD32(0x7100C200,tmp);
    tmp &= ~(0x1<<31);
    REG_WR32(0x7100C200,tmp);
    //HwUSB30->sGlobal.uGUSB3PIPECTL.bReg.PHYSoftRst = 0;
    REG_RD32(0x7100C2C0,tmp);
    tmp &= ~(0x1<<31);
    REG_WR32(0x7100C2C0,tmp);

    // Wait T3-T1 (>165us)
    wait_phy_reset_timing(4800*2);

    //HwUSB30->sGlobal.uGCTL.bReg.CoreSoftReset = 0;
    REG_RD32(0x7100C110,tmp);
    tmp &= ~(0x1<<11);
    REG_WR32(0x7100C110,tmp);
}

void wait_phy_reset_timing (unsigned int count)
{
    unsigned int i;
    volatile unsigned int tmp;

    // Wait 10us
    tmp = 0;
    for (i=0;i<count * 100;i++)
        tmp++;
}


void wait_phy_clock_valid(void)
{
    unsigned int uTmp;

    // Wait USB30 PHY initialization finish. PHY FREECLK should be stable.
    while(1) {
        // GDBGLTSSM : Check LTDB Sub State is non-zero
        // When PHY FREECLK is valid, LINK mac_resetn is released and LTDB Sub State change to non-zero
        //uTmp = HwUSB30->sGlobal.uGDBGLTSSM.bReg.LTDBSubState;
        REG_RD32(0x7100C164, uTmp);
        uTmp = (uTmp>>18)&0xF;

        // Break when LTDB Sub state is non-zero and CNR is zero
        if (uTmp != 0){
            break;
        }
    }
}

// USAGE EXAMPLES
void POWERON_RESET_EXAMPLE1 (void) {
    // USB30 MODE POWERON RESET EXAMPLE
    // USB30 Controller & PHY act as USB30 (SS/HS/FS)

    reset_usb30_poweron();
    set_mode_usb30();
    wait_phy_clock_valid();
}

void usb30_poweron_reset_usb20_mode (void) {
    // USB20 ONLY MODE POWERON RESET EXAMPLE
    // USB30 Controller & PHY act as only USB20 (HS/FS)

    reset_usb30_poweron();
    powerdown_ss();
    set_mode_usb20_only();
    wait_phy_clock_valid();
}

void POWER_TOGGLE_EXAMPLE1 (void) {
    // USB20 ONLY MODE POWERDOWN EXAMPLE

    powerdown_hs();
    poweron_hs();
    reset_usb30_poweron();
    set_mode_usb20_only();
    wait_phy_clock_valid();
}
