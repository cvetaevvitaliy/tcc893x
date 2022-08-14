#ifndef __LINUX_FT5X0X_TS_H__
#define __LINUX_FT5X0X_TS_H__

/* -- dirver configure -- */
#define CFG_MAX_TOUCH_POINTS	10

#define PRESS_MAX	0xFF
#define FT_PRESS	0x1

#define FT5X0X_NAME	"GT80X"

#define FT_MAX_ID	0x0F
#define FT_TOUCH_STEP	6
#define FT_TOUCH_X_H_POS		3
#define FT_TOUCH_X_L_POS		4
#define FT_TOUCH_Y_H_POS		5
#define FT_TOUCH_Y_L_POS		6
#define FT_TOUCH_EVENT_POS		3
#define FT_TOUCH_ID_POS			5

#define POINT_READ_BUF	(3 + FT_TOUCH_STEP * CFG_MAX_TOUCH_POINTS)

/*register address*/
#define FT5x0x_REG_FW_VER		0xA6
#define FT5x0x_REG_POINT_RATE	0x88
#define FT5X0X_REG_THGROUP	0x80

#define FT_MAX_X  1280
#define FT_MAX_Y  800

#define INT_GPIO	TCC_GPB(11)

#if defined(CONFIG_ARCH_TCC893X)
#define INT_IRQ		INT_EINT2
#else
#define INT_IRQ		INT_EI2
#endif
#define INT_CFG		EXTINT_GPIOB_11

#define RST_GPIO	TCC_GPB(7)




int ft5x0x_i2c_Read(struct i2c_client *client, char *writebuf, int writelen,
		    char *readbuf, int readlen);
int ft5x0x_i2c_Write(struct i2c_client *client, char *writebuf, int writelen);

/* The platform data for the Focaltech ft5x0x touchscreen driver */
struct ft5x0x_platform_data {
	unsigned int x_max;
	unsigned int y_max;
	unsigned long irqflags;
	unsigned int irq;
	unsigned int reset;
};

#endif
