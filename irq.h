#ifndef __UIIM_INTERUPT_H__
#define __UIIM_INTERUPT_H__
#include "gd32f4xx.h"
#define CAN_RX_QUEUE_SIZE 32U
extern volatile uint32_t can_rx_dropped[2];
extern volatile uint32_t can_rx_overruns[2];
int can_rx_pop(unsigned int channel, can_receive_message_struct *message);
void CAN0_RX0_IRQHandler(void);
void CAN1_RX0_IRQHandler(void);
#endif
