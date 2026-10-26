#include "ad8251.h"
#include "gd32f4xx.h"
#include "gd32f450z_eval.h"

void AD8251_INIT()
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN_14|GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_14|GPIO_PIN_15);
}

void AD8251_SET(int imod)
{
	if ((imod & 0x01)!=0)
	{
		__A0_S_SET();
	}
	else
	{
		__A0_S_CLR();
	};
	if ((imod & 0x02)!=0)
	{
		__A1_S_SET();
	}
	else
	{
		__A1_S_CLR();
	};
	return;
}

void AD8251_CLR()
{
		__A0_S_CLR();
		__A1_S_CLR();
}
