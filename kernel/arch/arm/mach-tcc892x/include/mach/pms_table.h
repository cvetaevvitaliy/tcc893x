/*
 *  P.M.S table list
 */

#ifndef __PMS_TABLE_H__
#define __PMS_TABLE_H__

typedef struct {
    unsigned int    uFpll;
    unsigned int    P;
    unsigned int    M;
    unsigned int    S;
    unsigned int    VSEL;
} sfPLL;

#define XIN_CLK_RATE    120000  // 12MHz
#define XTIN_CLK_RATE   327     // 32.768kHz
#define HDMI_CLK_RATE   270000  // 27MHz

#define PLLFREQ(P, M, S)        (( XIN_CLK_RATE * (M) )  / (P) ) >> (S) // 100Hz Unit..
#define FPLL_t(P, M, S, VSEL)   PLLFREQ(P,M,S), P, M, S, VSEL

static sfPLL pIO_CPU_PLL[] = {
    {FPLL_t(3, 300,  0,  0)},       // 1200 MHz
    {FPLL_t(3, 275,  0,  0)},       // 1100 MHz
//    {FPLL_t(3, 250,  0,  0)},       // 1000 MHz
    {FPLL_t(3, 500,  1,  1)},       // 1000 MHz
    {FPLL_t(3, 498,  1,  1)},       // 996 MHz
    {FPLL_t(3, 480,  1,  1)},       // 960
    {FPLL_t(3, 468,  1,  1)},       // 936
    {FPLL_t(3, 450,  1,  1)},       // 900
    {FPLL_t(3, 406,  1,  1)},       // 812
    {FPLL_t(3, 404,  1,  1)},       // 808
    {FPLL_t(3, 359,  1,  1)},       // 718
    {FPLL_t(3, 358,  1,  1)},       // 716
    {FPLL_t(3, 312,  1,  0)},       // 624
    {FPLL_t(3, 273,  1,  0)},       // 546
    {FPLL_t(3, 468,  2,  1)},       // 468
    {FPLL_t(3, 401,  2,  1)},       // 401
    {FPLL_t(3, 343,  2,  0)},       // 343
};
#define NUM_CPU_PLL                 (sizeof(pIO_CPU_PLL)/sizeof(sfPLL))

static sfPLL pIO_CKC_PLL[] = {
    {FPLL_t(3, 458,  1,  1)},       // 916    // mali
    {FPLL_t(3, 414,  1,  1)},       // 828    // mali
    {FPLL_t(3, 384,  1,  1)},       // 768
    {FPLL_t(3, 370,  1,  1)},       // 740    // mali
    {FPLL_t(3, 364,  1,  1)},       // 728
    {FPLL_t(3, 336,  1,  0)},       // 672
    {FPLL_t(3, 317,  1,  0)},       // 634    // mali
    {FPLL_t(3, 300,  1,  0)},       // 600
    {FPLL_t(3, 297,  1,  0)},       // 594    // composite, hdmi
    {FPLL_t(3, 264,  1,  0)},       // 528    // mali
    {FPLL_t(3, 500,  2,  1)},       // 500
    {FPLL_t(3, 448,  2,  1)},       // 448    // mali
    {FPLL_t(3, 368,  2,  1)},       // 368    // mali
};
#define NUM_PLL                 (sizeof(pIO_CKC_PLL)/sizeof(sfPLL))

#endif /*__PMS_TABLE_H__*/
