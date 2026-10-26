#include "sn74hc595d.h"
#include "gd32f4xx.h"
#include "gd32f450z_eval.h"
#include "ad8251.h"

//启动GPIOA, GPIOB, GPIOC
//分别设置6根引脚为输出，配置SRCLK1，SRCLK2为定时反转
void SN74HC595D_INIT()
{
	//============================================
	//配置GPIO时钟
	//启动GPIOA GPIOB GPIOC 时钟
	//============================================	
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	//============================================
	//配置GPIOC.05设置为输出
	//配置GPIOB.00设置为输出	
	//配置GPIOB.01设置为输出
	//配置GPIOA.04设置为输出
	//配置GPIOA.05设置为输出	
	//配置GPIOA.07设置为输出
	gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN_5);
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN_0|GPIO_PIN_1);	
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7);

  gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_5);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_0|GPIO_PIN_1);	
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7);
	return;
}

void SN74HC595D_RUN(unsigned int data)
{
	RUN_CH1(data);
	RUN_CH2(data);
}

void RUN_CH1(unsigned int data)
{
	int i;
	SRCLK1_CLR();
	RCLK1_CLR();
	for(i=0;i<8;i++)
	{
		if ((data&0x80)==0x80)
		{
			DS1_SET();
		}
		else
		{
			DS1_CLR();
		}
		SRCLK1_SET();
		SRCLK1_CLR();
		data = data << 1;
	}
	RCLK1_SET();
	RCLK1_CLR();
}

void RUN_CH2(unsigned int data)
{
	int i;
	SRCLK2_CLR();
	RCLK2_CLR();
	for(i=0;i<8;i++)
	{
		if ((data&0x80)==0x80)
		{
			DS2_SET();
		}
		else
		{
			DS2_CLR();
		}
		SRCLK2_SET();
		SRCLK2_CLR();
		data = data << 1;
	}
	RCLK2_SET();
	RCLK2_CLR();
}

void CLR_CH1(void)
{
	SRCLK1_CLR();
	RCLK1_CLR();
	DS1_CLR();
}

void CLR_CH2(void)
{
	SRCLK2_CLR();
	RCLK2_CLR();
	DS2_CLR();	
}
