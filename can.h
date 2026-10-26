#ifndef __MY_CAN_H__
#define __MY_CAN_H__
#include "gd32f4xx.h"
#include "gd32f4xx_can.h"
#define CAN_TX_QUEUE_SIZE 16U
extern uint32_t can_tx_dropped[2], can_tx_failed[2], can_tx_timeouts[2];
extern unsigned int can_init_ok;
void MY_CAN_INIT(void);
/* Main-loop only: 1 = copied to queue, 0 = invalid/full.
   Queue acceptance is NOT a physical-bus acknowledgement. */
unsigned int send_can(unsigned char channel, unsigned int id,
                      const unsigned char *data, unsigned char length);
unsigned int can_tx_free(unsigned int channel);
void can_tx_poll(uint32_t now);
unsigned int can_channel_error_get(unsigned char channel);
#endif
