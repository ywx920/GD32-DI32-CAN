#include "irq.h"
#include "gd32f4xx_can.h"
static can_receive_message_struct rx_queue[2][CAN_RX_QUEUE_SIZE];
static volatile unsigned int rx_head[2], rx_tail[2];
volatile uint32_t can_rx_dropped[2], can_rx_overruns[2];
static void receive_irq(unsigned int channel, uint32_t peripheral)
{
    unsigned int next;
    can_receive_message_struct message;
    while (can_receive_message_length_get(peripheral, CAN_FIFO0) != 0U) {
        can_message_receive(peripheral, CAN_FIFO0, &message);
        if (message.rx_ff != CAN_FF_EXTENDED || message.rx_ft != CAN_FT_DATA) {
            continue;
        }
        next = (rx_head[channel] + 1U) % CAN_RX_QUEUE_SIZE;
        if (next == rx_tail[channel]) {
            ++can_rx_dropped[channel];
        } else {
            rx_queue[channel][rx_head[channel]] = message;
            __DMB();
            rx_head[channel] = next;
        }
    }
    if (can_flag_get(peripheral, CAN_FLAG_RFO0) != RESET) {
        ++can_rx_overruns[channel];
        can_flag_clear(peripheral, CAN_FLAG_RFO0);
    }
}
int can_rx_pop(unsigned int channel, can_receive_message_struct *message)
{
    uint32_t saved;
    if (channel > 1U || message == 0) { return 0; }
    saved = __get_PRIMASK();
    __disable_irq();
    if (rx_head[channel] == rx_tail[channel]) {
        __set_PRIMASK(saved); return 0;
    }
    *message = rx_queue[channel][rx_tail[channel]];
    rx_tail[channel] = (rx_tail[channel] + 1U) % CAN_RX_QUEUE_SIZE;
    __set_PRIMASK(saved);
    return 1;
}
void CAN0_RX0_IRQHandler(void) { receive_irq(0U, CAN0); }
void CAN1_RX0_IRQHandler(void) { receive_irq(1U, CAN1); }
