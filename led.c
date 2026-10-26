#include "led.h"
#include "gd32f4xx.h"
#include "gd32f450z_eval.h"

unsigned int ICAN_LED2;
unsigned int ICAN_LED1;
unsigned int ICAN_LED2_OFF;
unsigned int ICAN_LED1_OFF;

void MY_LED_INIT(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);

    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                  LED_CAN1 | LED_CAN2 | LED_FAU);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            LED_CAN1 | LED_CAN2 | LED_FAU);

    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_RUN);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED_RUN);
	
		MY_LED_CLR();
}

void MY_LED_RUN(void)
{
	LED_CAN2_SET();
	LED_CAN1_SET();
	LED_RUN_SET();
	//LED_FAU_SET();
}

void MY_LED_CLR(void)
{
	LED_CAN2_CLR();
	LED_CAN1_CLR();
	LED_RUN_CLR();
	LED_FAU_CLR();
}
