#ifndef __LED_H__
#define __LED_H__

/* Both colors of the run/fault bicolor LED are active high. */
//LED_RUN  -- J3.5  -- PC1
//LED_FAU  -- J3.7  -- PA1 (SYS_LED_L)
//LED_CAN1 -- J3.11 -- PA4
//LED_CAN2 -- J3.13 -- PA5

#define LED_CAN2 GPIO_PIN_5
#define LED_RUN  GPIO_PIN_1
#define LED_CAN1 GPIO_PIN_4
#define LED_FAU  GPIO_PIN_1

#define LED_CAN2_SET() gpio_bit_set(GPIOA,LED_CAN2)
#define LED_CAN2_CLR() gpio_bit_reset(GPIOA,LED_CAN2)
#define LED_CAN2_GET() gpio_input_bit_get(GPIOA,LED_CAN2)

#define LED_CAN1_SET() gpio_bit_set(GPIOA,LED_CAN1)
#define LED_CAN1_CLR() gpio_bit_reset(GPIOA,LED_CAN1)
#define LED_CAN1_GET() gpio_input_bit_get(GPIOA,LED_CAN1)

#define LED_RUN_SET() gpio_bit_set(GPIOC,LED_RUN)
#define LED_RUN_CLR() gpio_bit_reset(GPIOC,LED_RUN)
#define LED_RUN_GET() gpio_input_bit_get(GPIOC,LED_RUN)

#define LED_FAU_SET() gpio_bit_set(GPIOA,LED_FAU)
#define LED_FAU_CLR() gpio_bit_reset(GPIOA,LED_FAU)

extern unsigned int ICAN_LED2;
extern unsigned int ICAN_LED1;
extern unsigned int ICAN_LED2_OFF;
extern unsigned int ICAN_LED1_OFF;

extern void MY_LED_INIT(void);
extern void MY_LED_RUN(void);
extern void MY_LED_CLR(void);

#endif
