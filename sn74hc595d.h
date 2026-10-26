#ifndef __SN74HC595D_H__
#define __SN74HC595D_H__

//CT1.18 --> S_IO_4 --> PC5 = SER1		:: DS
//CT1.19 --> S_IO_5 --> PB0 = SRCLK1	::SHCP
//CT1.20 --> S_IO_6 --> PB1 = RCLK1		::STCP

//CT1.12 --> DAC0   --> PA4 = SRCLK2	::SHCP
//CT1.14 --> DAC1   --> PA5 = RCLK2	  ::STCP
//CT1.16 --> S_IO_2 --> PA7 = SER2		:: DS

//启动GPIOA, GPIOB, GPIOC,分别设置模式为输出

#define __DS01__   GPIO_PIN_5
#define __SCLK01__ GPIO_PIN_0
#define __RCLK01__ GPIO_PIN_6

#define __DS02__   GPIO_PIN_7
#define __SCLK02__ GPIO_PIN_4
#define __RCLK02__ GPIO_PIN_5

#define DS1_CLR() gpio_bit_reset(GPIOC,__DS01__)
#define DS1_SET() gpio_bit_set(GPIOC,__DS01__)
#define SRCLK1_CLR() gpio_bit_reset(GPIOB,__SCLK01__)
#define SRCLK1_SET() gpio_bit_set(GPIOB,__SCLK01__)
#define RCLK1_CLR() gpio_bit_reset(GPIOB,__RCLK01__)
#define RCLK1_SET() gpio_bit_set(GPIOB,__RCLK01__)

#define DS2_CLR() gpio_bit_reset(GPIOA,__DS02__)
#define DS2_SET() gpio_bit_set(GPIOA,__DS02__)
#define SRCLK2_CLR() gpio_bit_reset(GPIOA,__SCLK02__)
#define SRCLK2_SET() gpio_bit_set(GPIOA,__SCLK02__)
#define RCLK2_CLR() gpio_bit_reset(GPIOA,__RCLK02__)
#define RCLK2_SET() gpio_bit_set(GPIOA,__RCLK02__)

extern void SN74HC595D_INIT(void);
extern void SN74HC595D_RUN(unsigned int data);
extern void RUN_CH1(unsigned int data);
extern void RUN_CH2(unsigned int data);
extern void CLR_CH1(void);
extern void CLR_CH2(void);

#endif
