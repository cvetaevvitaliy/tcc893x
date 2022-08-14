#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <mach/irqs.h>
#include <mach/bsp.h>
#include <asm/mach-types.h>
#include <asm/system.h>


#if defined(CONFIG_CHIP_TCC8930)
static int uart_port_map[40][5] = {
//    tx			   rx			     rts			cts			  fn
	{TCC_GPA(26), TCC_GPA(27), TCC_GPA(24), TCC_GPA(25), GPIO_FN(4)},   // 0  // UT_TXD(0)
	{TCC_GPA(28), TCC_GPA(29), TCC_GPA(21), TCC_GPA(20), GPIO_FN(4)},   // 1  // UT_TXD(1)
	{TCC_GPA(13), TCC_GPA(14),                  0,                  0, GPIO_FN(7)},   // 2  // UT_TXD(1)
	{TCC_GPA(24), TCC_GPA(25), TCC_GPA(22), TCC_GPA(23), GPIO_FN(5)},   // 3  // UT_TXD(2)
	{TCC_GPD(4),   TCC_GPD(5),  TCC_GPD(6),   TCC_GPD(7),  GPIO_FN(7)},   // 4  // UT_TXD(2)
	{TCC_GPD(11), TCC_GPD(12), TCC_GPD(13), TCC_GPD(14), GPIO_FN(7)},   // 5  // UT_TXD(3)
	{TCC_GPD(22), TCC_GPD(23), TCC_GPD(24), TCC_GPD(25), GPIO_FN(14)}, // 6  // UT_TXD(3)	
	{TCC_GPD(13), TCC_GPD(14),                  0,                   0, GPIO_FN(4)},  // 7  // UT_TXD(3)	
	{TCC_GPD(17), TCC_GPD(18), TCC_GPD(19), TCC_GPD(20), GPIO_FN(7)},   // 8 // UT_TXD(4)
	{TCC_GPB(7),   TCC_GPB(8),  TCC_GPB(9),   TCC_GPB(10), GPIO_FN(10)}, // 9  // UT_TXD(5)
	{TCC_GPB(11), TCC_GPB(12), TCC_GPB(13), TCC_GPB(14), GPIO_FN(10)}, // 10	// UT_TXD(6)
	{TCC_GPB(19), TCC_GPB(20), TCC_GPB(21), TCC_GPB(22), GPIO_FN(10)}, // 11  // UT_TXD(7)
	{TCC_GPB(25), TCC_GPB(26), TCC_GPB(27), TCC_GPB(28), GPIO_FN(10)}, // 12  // UT_TXD(8)
	{TCC_GPC(14), TCC_GPC(15), TCC_GPC(16), TCC_GPC(17), GPIO_FN(6)},  // 13  // UT_TXD(9)
	{TCC_GPC(22), TCC_GPC(23), TCC_GPC(24), TCC_GPC(25), GPIO_FN(6)},  // 14   // UT_TXD(10)
	{TCC_GPC(1),   TCC_GPC(2),                   0,                  0, GPIO_FN(6)},   // 15  // UT_TXD(10)	
	{TCC_GPC(28), TCC_GPC(29), TCC_GPC(30), TCC_GPC(31), GPIO_FN(6)},  // 16   // UT_TXD(11)
	{TCC_GPC(10), TCC_GPC(11),                  0,                  0, GPIO_FN(15)}, // 17   // UT_TXD(11)	
	{TCC_GPC(16), TCC_GPC(17),                  0,                  0, GPIO_FN(7)},  // 18   // UT_TXD(11)		
	{TCC_GPE(13), TCC_GPE(14), TCC_GPE(15), TCC_GPE(16), GPIO_FN(5)},  // 19   // UT_TXD(12)
	{TCC_GPE(11), TCC_GPE(12), 			   0,                  0, GPIO_FN(5)}, // 20    // UT_TXD(12)	
	{TCC_GPE(15), TCC_GPE(16), 			   0,                  0, GPIO_FN(9)}, // 21    // UT_TXD(12)		
	{TCC_GPE(20), TCC_GPE(18), 			   0,                  0, GPIO_FN(15)}, // 22    // UT_TXD(12)		
	{TCC_GPE(30), TCC_GPE(31), TCC_GPE(28), TCC_GPE(29), GPIO_FN(5)},   // 23  // UT_TXD(13)
	{TCC_GPE(28), TCC_GPE(29),                   0,                  0, GPIO_FN(5)},  // 24   // UT_TXD(13)	
	{TCC_GPF(13), TCC_GPF(14), TCC_GPF(15), TCC_GPF(16), GPIO_FN(9)},   // 25  // UT_TXD(14) 
	{TCC_GPF(25), TCC_GPF(26),                   0,                  0, GPIO_FN(15)}, // 26    // UT_TXD(14) 	
	{TCC_GPF(17), TCC_GPF(18), TCC_GPF(19), TCC_GPF(20), GPIO_FN(9)},   // 27  // UT_TXD(15)
	{TCC_GPF(30), TCC_GPF(31), TCC_GPF(28), TCC_GPF(29), GPIO_FN(9)},   // 28  // UT_TXD(16)
	{TCC_GPG(11), TCC_GPG(12), TCC_GPG(13), TCC_GPG(14), GPIO_FN(3)},  // 29   // UT_TXD(17)
	{TCC_GPG(15), TCC_GPG(16),                  0,                  0, GPIO_FN(3)},  // 30   // UT_TXD(18)
	{TCC_GPG(17), TCC_GPG(19),                  0,                  0, GPIO_FN(3)},  // 31   // UT_TXD(19)
	{TCC_GPG(15), TCC_GPG(14), TCC_GPG(13), TCC_GPG(12), GPIO_FN(15)},  // 32   // UT_RXD(20) 
	{TCC_GPG(19), TCC_GPG(18), TCC_GPG(17), TCC_GPG(16), GPIO_FN(15)},  // 33   // UT_RXD(20) 
	{TCC_GPG(10), TCC_GPG(9),   TCC_GPG(8),   TCC_GPG(7),  GPIO_FN(3)},  // 34   // UT_RXD(20) 	
	{TCC_GPG(14), TCC_GPG(13),                  0,                  0, GPIO_FN(5)},  // 35   // UT_RXD(20) 
	{TCC_GPG(5),  TCC_GPG(6),                    0,                  0, GPIO_FN(3)},  // 36	     // UT_RXD(21) 
	{TCC_GPG(14), TCC_GPG(13),                  0,                  0, GPIO_FN(6)},  // 37	     // UT_RXD(21) 
	{TCC_GPHDMI(2), TCC_GPHDMI(3), TCC_GPHDMI(0), TCC_GPHDMI(1), GPIO_FN(3)},  // 38   // UT_TXD(22)		
	{TCC_GPADC(4), TCC_GPADC(5), TCC_GPADC(3), TCC_GPADC(2), GPIO_FN(2)},    // 39  // UT_TXD(23)
};
#elif defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
static int uart_port_map[27][5] = {
//    tx			   rx			     rts			cts			  fn
	{TCC_GPA(7), TCC_GPA(8), TCC_GPA(9), TCC_GPA(10), GPIO_FN(7)},   // 0  // UT_TXD(0)
	{TCC_GPA(13), TCC_GPA(14), TCC_GPA(15), TCC_GPA(16), GPIO_FN(7)},   // 1  // UT_TXD(1)
	{TCC_GPD(4),   TCC_GPD(5),  TCC_GPD(6),   TCC_GPD(7),  GPIO_FN(7)},   // 2  // UT_TXD(2)
	{TCC_GPD(11), TCC_GPD(12), TCC_GPD(13), TCC_GPD(14), GPIO_FN(7)},   // 3  // UT_TXD(3)
	{TCC_GPD(17), TCC_GPD(18), TCC_GPD(19), TCC_GPD(20), GPIO_FN(7)},   // 4  // UT_TXD(4)	
	{TCC_GPB(0),   TCC_GPB(1),  TCC_GPB(2),   TCC_GPB(3), GPIO_FN(10)}, // 5  // UT_TXD(5)
	{TCC_GPB(11), TCC_GPB(12), TCC_GPB(13), TCC_GPB(14), GPIO_FN(10)}, // 6	// UT_TXD(6)
	{TCC_GPB(19), TCC_GPB(20), TCC_GPB(21), TCC_GPB(22), GPIO_FN(10)}, // 7  // UT_TXD(7)
	{TCC_GPB(25), TCC_GPB(26), TCC_GPB(27), TCC_GPB(28), GPIO_FN(10)}, // 8  // UT_TXD(8)
	{TCC_GPC(0),  TCC_GPC(1),  TCC_GPC(2),  TCC_GPC(3),  GPIO_FN(6)},   // 9  // UT_TXD(10)	
	{TCC_GPC(10), TCC_GPC(11), TCC_GPC(12), TCC_GPC(13), GPIO_FN(6)}, // 10   // UT_TXD(11)	
	{TCC_GPE(12), TCC_GPE(13), TCC_GPE(14), TCC_GPE(15), GPIO_FN(5)},  // 11   // UT_TXD(12)
	{TCC_GPF(11), TCC_GPF(12), TCC_GPF(13), TCC_GPF(14), GPIO_FN(9)},   // 12  // UT_TXD(14) 
	{TCC_GPF(25), TCC_GPF(26), TCC_GPF(27), TCC_GPF(28), GPIO_FN(9)}, // 13    // UT_TXD(16) 	
	{TCC_GPG(0),  TCC_GPG(1),  TCC_GPG(2),  TCC_GPG(3), GPIO_FN(3)},  // 14   // UT_TXD(17)
	{TCC_GPG(4),  TCC_GPG(5),  TCC_GPG(6),  TCC_GPG(7), GPIO_FN(3)},  // 15   // UT_TXD(18)
	{TCC_GPG(8),  TCC_GPG(9),  TCC_GPG(10), TCC_GPG(11), GPIO_FN(3)},  // 16   // UT_TXD(19)
	{TCC_GPG(15), TCC_GPG(14), TCC_GPG(13), TCC_GPG(12), GPIO_FN(3)},  // 17   // UT_RXD(20) 
	{TCC_GPG(19), TCC_GPG(18), TCC_GPG(17), TCC_GPG(16), GPIO_FN(3)},  // 18   // UT_RXD(21) 
	{TCC_GPHDMI(0), TCC_GPHDMI(1), TCC_GPHDMI(2), TCC_GPHDMI(3), GPIO_FN(3)},  // 19   // UT_TXD(22)		
	{TCC_GPADC(2), TCC_GPADC(3),              0,                  0, GPIO_FN(2)},    // 20  // UT_TXD(23)
};
#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933) 
static int uart_port_map[27][5] = {
//    tx			   rx			     rts			cts			  fn
	{TCC_GPA(13), TCC_GPA(14),                  0,                  0, GPIO_FN(7)},   // 0  // UT_TXD(1)
	{TCC_GPD(4),   TCC_GPD(5),  TCC_GPD(6),   TCC_GPD(7),  GPIO_FN(7)},   // 1  // UT_TXD(2)
	{TCC_GPD(11), TCC_GPD(12), TCC_GPD(13), TCC_GPD(14), GPIO_FN(14)},   // 2  // UT_TXD(3)
	{TCC_GPB(7),   TCC_GPB(8),  TCC_GPB(9),   TCC_GPB(10), GPIO_FN(10)}, // 3  // UT_TXD(5)
	{TCC_GPB(11), TCC_GPB(12), TCC_GPB(13), TCC_GPB(14), GPIO_FN(10)}, // 4	// UT_TXD(6)
	{TCC_GPB(19), TCC_GPB(20), TCC_GPB(21), TCC_GPB(22), GPIO_FN(10)}, // 5  // UT_TXD(7)
	{TCC_GPB(25), TCC_GPB(26), TCC_GPB(27), TCC_GPB(28), GPIO_FN(10)}, // 6  // UT_TXD(8)
	{TCC_GPC(1),   TCC_GPC(20),                   0,                  0, GPIO_FN(6)},   // 7  // UT_TXD(10)	
	{TCC_GPC(10), TCC_GPC(11),                  0,                  0, GPIO_FN(15)}, // 8   // UT_TXD(11)	
	{TCC_GPC(16), TCC_GPC(17),                  0,                  0, GPIO_FN(7)},  // 9   // UT_TXD(11)		
	{TCC_GPE(13), TCC_GPE(14), TCC_GPE(15), TCC_GPE(16), GPIO_FN(5)},  // 10   // UT_TXD(12)
	{TCC_GPE(20), TCC_GPE(18), 			   0,                  0, GPIO_FN(15)}, // 11    // UT_TXD(12)	
	{TCC_GPE(15), TCC_GPE(16), 			   0,                  0, GPIO_FN(9)}, // 12    // UT_TXD(12)		
	{TCC_GPE(28), TCC_GPE(27),                   0,                  0, GPIO_FN(6)},  // 13   // UT_TXD(13) // GPIO_E28 -> RX?
	{TCC_GPF(13), TCC_GPF(14), TCC_GPF(15), TCC_GPF(16), GPIO_FN(9)},   // 14  // UT_TXD(14) 
	{TCC_GPF(25), TCC_GPF(26),                   0,                  0, GPIO_FN(15)}, // 15    // UT_TXD(14) 	
	{TCC_GPG(11), TCC_GPG(12), TCC_GPG(13), TCC_GPG(14), GPIO_FN(3)},  // 16   // UT_TXD(17)
	{TCC_GPG(15), TCC_GPG(16),                  0,                  0, GPIO_FN(3)},  // 17   // UT_TXD(18)
	{TCC_GPG(17), TCC_GPG(19),                  0,                  0, GPIO_FN(3)},  // 18   // UT_TXD(19)
	{TCC_GPG(15), TCC_GPG(14), TCC_GPG(13), TCC_GPG(12), GPIO_FN(15)},  // 19   // UT_RXD(20) 
	{TCC_GPG(19), TCC_GPG(18), TCC_GPG(17), TCC_GPG(16), GPIO_FN(15)},  // 20   // UT_RXD(20) 
	{TCC_GPG(10), TCC_GPG(9),   TCC_GPG(8),   TCC_GPG(7),  GPIO_FN(3)},  // 21   // UT_RXD(20) 
	{TCC_GPG(14), TCC_GPG(13),                  0,                  0, GPIO_FN(5)},  // 22   // UT_RXD(20) 
	{TCC_GPG(5),  TCC_GPG(6),                    0,                  0, GPIO_FN(3)},  // 23	     // UT_RXD(21) 
	{TCC_GPG(14), TCC_GPG(13),                  0,                  0, GPIO_FN(6)},  // 24	     // UT_RXD(21) 
	{TCC_GPHDMI(2), TCC_GPHDMI(3), TCC_GPHDMI(0), TCC_GPHDMI(1), GPIO_FN(3)},  // 25   // UT_TXD(22)		
	{TCC_GPADC(4), TCC_GPADC(5), TCC_GPADC(3), TCC_GPADC(2), GPIO_FN(2)},    // 26  // UT_TXD(23)
};
#endif

int uart_port_map_selection[8][5];
EXPORT_SYMBOL(uart_port_map_selection);

static int __init m805_893x_init_uart_map(void)
{
	int i;
	printk("%s\n",__func__);

	for(i=0; i<8; i++) //initial data [0]
		uart_port_map_selection[i][0] = -1;
	
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
	for (i=0 ; i<5; i++) {
		uart_port_map_selection[0][i] = uart_port_map[2][i]; // console(ttyTCC0)
//		uart_port_map_selection[1][i] = uart_port_map[17][i]; // BT(ttyTCC1)
	}
#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8933)
	for (i=0 ; i<5; i++) {
		uart_port_map_selection[0][i] = uart_port_map[1][i]; // console(ttyTCC0)
//		uart_port_map_selection[1][i] = uart_port_map[19][i]; // BT(ttyTCC1)
	}
#endif


}

device_initcall(m805_893x_init_uart_map);
