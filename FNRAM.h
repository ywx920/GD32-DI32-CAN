//**************************************************************************************
//								此头文件用于FM24L256  for LPC1768
//							  	I2C铁电存储器 
//								V1:	owen 2013.09.17
//								V2:	2016-04-06 20:36:32
//**************************************************************************************
#ifndef _FNRAM_
#define _FNRAM_

#include "gd32f4xx.h"
#include <stdio.h>
#include "gd32f450z_eval.h"
#include <string.h>
typedef unsigned short int uint16;
typedef unsigned char uint8;
typedef unsigned int uint32;
/*
#define FM_WP_MOVE		    (1 << 29)  	 //P0.29
#define FM_SCL_MOVE 	    (1 << 28)    //P0.28	
#define FM_SDA_MOVE		    (1 << 27)    //P0.27

#define FM_WP_IO_CON		LPC_GPIO0
#define FM_SCL_IO_CON		LPC_GPIO0		
#define FM_SDA_IO_CON		LPC_GPIO0
*/
//存储芯片 FM24L256
//最大地址 0x7FFF ~ 0x0000
#define SET_SCL_FM  	(GPIO_BOP(GPIOB) = GPIO_PIN_10)//scl			
#define CLR_SCL_FM		(GPIO_BC(GPIOB) = GPIO_PIN_10)			
#define SET_SDA_FM  	(GPIO_BOP(GPIOB) = GPIO_PIN_11)//sda			
#define CLR_SDA_FM  	(GPIO_BC(GPIOB) = GPIO_PIN_11)			
//#define SET_WP_FM   	FM_WP_IO_CON->FIOSET = FM_WP_MOVE			
//#define CLR_WP_FM   	FM_WP_IO_CON->FIOCLR = FM_WP_MOVE			

#define DATA_OUT_FM 	(gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN_11))		
#define DATA_IN_FM  	(gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE,GPIO_PIN_11))		

#define READ_IO_FM  	(gpio_input_bit_get(GPIOB,GPIO_PIN_11))		// due to hardware

#define FM_ADDRESS      0xA0   //铁电存储器的IIC总线器件地址

//#define WRITE_FM_ENABLE  CLR_WP_FM
//#define WRITE_FM_DISABLE SET_WP_FM

void FNRAM_init(void);  //管脚初始化



/*******************************************************************
                    ROM存储器读写操作             
功能: 		ROM存储器读写操作
参数定义：	suba：将要读或写数据的器件子地址首地址
			s：	  发送时，代表将要发的数据区的首地址
				  接收时，代表接收到的数据保存至的首地址
			num： 接收或发送的数据个数
********************************************************************/
uint8 FM_SendStr(uint16 suba, uint8 *s, uint8 num);

uint8 FM_ReadStr(uint16 suba, uint8 *s, uint8 num);
#endif





