#ifndef __AD8251_H__
#define __AD8251_H__
#include <gd32f4xx_gpio.h>
//A0_S -- S_IO_7 -- PB15
//A1_S -- S_IO_8 -- PB14


#define __S_IO_7 GPIO_PIN_15
#define __S_IO_8 GPIO_PIN_14;

#define __A1_S_SET() gpio_bit_set(GPIOB, GPIO_PIN_14)
#define __A1_S_CLR() gpio_bit_reset(GPIOB, GPIO_PIN_14)

#define __A0_S_SET() gpio_bit_set(GPIOB, __S_IO_7)
#define __A0_S_CLR() gpio_bit_reset(GPIOB, __S_IO_7)


extern void AD8251_INIT(void);
extern void AD8251_SET(int imod);
extern void AD8251_CLR(void);

#endif
