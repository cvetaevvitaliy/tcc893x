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

//#define XIN_CLK_RATE    120000    // 12MHz
#define XIN_CLK_RATE    240000  // 24MHz
#define XTIN_CLK_RATE   327     // 32.768kHz
#define HDMI_CLK_RATE   270000

#define PLLFREQ(P, M, S)        (( XIN_CLK_RATE * (M) )  / (P) ) >> (S) // 100Hz Unit..
#define FPLL_t(P, M, S, VSEL)   PLLFREQ(P,M,S), P, M, S, VSEL

static sfPLL pIO_CPU_PLL[] = {
    {FPLL_t(6, 300,  0,  0)},       // 1200
    {FPLL_t(6, 288,  0,  0)},       // 1152
    {FPLL_t(6, 278,  0,  0)},       // 1112
    {FPLL_t(6, 265,  0,  0)},       // 1060
    {FPLL_t(6, 500,  1,  1)},       // 1000
    {FPLL_t(6, 498,  1,  1)},       // 996
    {FPLL_t(6, 455,  1,  1)},       // 910
    {FPLL_t(6, 446,  1,  1)},       // 892
    {FPLL_t(6, 425,  1,  1)},       // 850
    {FPLL_t(6, 364,  1,  1)},       // 728
    {FPLL_t(6, 313,  1,  0)},       // 626
    {FPLL_t(6, 262,  1,  0)},       // 524
    {FPLL_t(6, 424,  2,  1)},       // 424
    {FPLL_t(6, 342,  2,  0)},       // 342
};
#define NUM_CPU_PLL                 (sizeof(pIO_CPU_PLL)/sizeof(sfPLL))

static sfPLL pIO_CKC_PLL[] = {
    {FPLL_t(6, 500,  1,  1)},       // 1000
    {FPLL_t(6, 384,  1,  1)},       // 768
    {FPLL_t(6, 360,  1,  1)},       // 720
    {FPLL_t(6, 307,  1,  0)},       // 614
    {FPLL_t(6, 300,  1,  0)},       // 600
    {FPLL_t(6, 297,  1,  0)},       // 594
    {FPLL_t(6, 284,  1,  0)},       // 568
    {FPLL_t(6, 256,  1,  0)},       // 512
    {FPLL_t(6, 500,  2,  1)},       // 500
    {FPLL_t(6, 375,  2,  1)},       // 375
    {FPLL_t(6, 263,  2,  0)},       // 263
    {FPLL_t(6, 402,  3,  1)},       // 201
};
#define NUM_PLL                 (sizeof(pIO_CKC_PLL)/sizeof(sfPLL))

#endif /*__PMS_TABLE_H__*/
