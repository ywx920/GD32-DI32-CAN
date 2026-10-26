#include "can.h"
#include "irq.h"
#include "led.h"
#include <string.h>

static can_transmit_message_struct tx_queue[2][CAN_TX_QUEUE_SIZE];
static unsigned int tx_head[2], tx_tail[2];
static uint8_t mailbox[2] = {CAN_NOMAILBOX, CAN_NOMAILBOX};
static uint8_t attempts[2];
static uint32_t started_ms[2], last_ms[2];
static uint32_t gap_ms[2] = {1U, 1U};
static unsigned int link_error[2];
uint32_t can_tx_dropped[2], can_tx_failed[2], can_tx_timeouts[2];
unsigned int can_init_ok;

static void gpio_config(void)
{
	rcu_periph_clock_enable(RCU_SYSCFG);
    /* enable can clock */
    rcu_periph_clock_enable(RCU_CAN0);
	rcu_periph_clock_enable(RCU_GPIOA);

	rcu_periph_clock_enable(RCU_CAN1);
    rcu_periph_clock_enable(RCU_GPIOB);
    

	/* configure CAN0 GPIO, CAN0_RX(PA11) and CAN0_TX(PA12) */
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
    gpio_af_set(GPIOA, GPIO_AF_9, GPIO_PIN_11);
    
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_af_set(GPIOA, GPIO_AF_9, GPIO_PIN_12);



	/* configure CAN1 GPIO, CAN1_TX(PB13) and CAN1_RX(PB12) */
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_13);
    
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_12);

}
static void can_networking_init(void)
{
    can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;
		can_filter_parameter_struct can_filter1;
    uint32_t clock;
    can_init_ok = 0U;
    clock = rcu_clock_freq_get(CK_APB1);
    if ((clock % 2500000U) != 0U) { return; }
    /* initialize CAN register */
    can_deinit(CAN0);
    can_deinit(CAN1);

    /* initialize CAN */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_5TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_4TQ;
    /* 250 kbit/s: APB1 / (prescaler * 10 TQ) */
    can_parameter.prescaler = (uint16_t)(clock / 2500000U);
    if (can_init(CAN0, &can_parameter) != SUCCESS) { return; }
		if (can_init(CAN1, &can_parameter) != SUCCESS) { return; }

		can_filter.filter_number = 0;
    /* initialize filter */    
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0004;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0006;  
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;
    can_filter_init(&can_filter);
		can_interrupt_enable(CAN0, CAN_INTEN_RFNEIE0);

		can_filter1.filter_number = 15;
    /* initialize filter */    
    can_filter1.filter_mode = CAN_FILTERMODE_MASK;
    can_filter1.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter1.filter_list_high = 0x0000;
    can_filter1.filter_list_low = 0x0004;
    can_filter1.filter_mask_high = 0x0000;
    can_filter1.filter_mask_low = 0x0006;  
    can_filter1.filter_fifo_number = CAN_FIFO0;
    can_filter1.filter_enable = ENABLE;
    can_filter_init(&can_filter1);

		can_interrupt_enable(CAN1, CAN_INTEN_RFNEIE0);
		nvic_irq_enable(CAN0_RX0_IRQn,0,0);
    nvic_irq_enable(CAN1_RX0_IRQn,0,0);
    can_init_ok = 1U;
}


void MY_CAN_INIT(void)
{
    gpio_config();
    can_networking_init();
}
unsigned int can_tx_free(unsigned int channel)
{
    if (channel > 1U) { return 0U; }
    return (tx_tail[channel] + CAN_TX_QUEUE_SIZE - tx_head[channel] - 1U)
           % CAN_TX_QUEUE_SIZE;
}
unsigned int send_can(unsigned char channel, unsigned int id,
                      const unsigned char *data, unsigned char length)
{
    can_transmit_message_struct *message;
    if (channel > 1U || id > 0x1FFFFFFFU || length > 8U || data == 0) {
        return 0U;
    }
    if (can_tx_free(channel) == 0U) { ++can_tx_dropped[channel]; return 0U; }
    message = &tx_queue[channel][tx_head[channel]];
    memset(message, 0, sizeof(*message));
    message->tx_efid = id;
    message->tx_ff = CAN_FF_EXTENDED;
    message->tx_ft = CAN_FT_DATA;
    message->tx_dlen = length;
    memcpy(message->tx_data, data, length);
    tx_head[channel] = (tx_head[channel] + 1U) % CAN_TX_QUEUE_SIZE;
    return 1U;
}
void can_tx_poll(uint32_t now)
{
    unsigned int ch;
    uint32_t peripheral;
    can_transmit_state_enum state;
    if (!can_init_ok) { return; }
    for (ch = 0U; ch < 2U; ++ch) {
        peripheral = ch == 0U ? CAN0 : CAN1;
        if (mailbox[ch] != CAN_NOMAILBOX) {
            state = can_transmit_states(peripheral, mailbox[ch]);
            if (state == CAN_TRANSMIT_PENDING &&
                (uint32_t)(now - started_ms[ch]) < 100U) { continue; }
            if (state == CAN_TRANSMIT_PENDING) {
                can_transmission_stop(peripheral, mailbox[ch]);
                ++can_tx_timeouts[ch];
            }
            mailbox[ch] = CAN_NOMAILBOX;
            last_ms[ch] = now;
            if (state == CAN_TRANSMIT_OK) {
                link_error[ch] = CAN_ERROR_NONE;
                tx_tail[ch] = (tx_tail[ch] + 1U) % CAN_TX_QUEUE_SIZE;
                attempts[ch] = 0U;
                gap_ms[ch] = 1U;
            } else {
                link_error[ch] = CAN_ERROR_ACK;
                ++can_tx_failed[ch];
                gap_ms[ch] = 10U;
                if (attempts[ch] >= 3U) {
                    tx_tail[ch] = (tx_tail[ch] + 1U) % CAN_TX_QUEUE_SIZE;
                    attempts[ch] = 0U;
                    ++can_tx_dropped[ch];
                }
            }
        }
        if (tx_tail[ch] == tx_head[ch] ||
            (uint32_t)(now - last_ms[ch]) < gap_ms[ch]) { continue; }
        mailbox[ch] = can_message_transmit(peripheral, &tx_queue[ch][tx_tail[ch]]);
        if (mailbox[ch] != CAN_NOMAILBOX) {
            started_ms[ch] = now;
            ++attempts[ch];
            if (ch == 0U) { ICAN_LED1 = 1U; } else { ICAN_LED2 = 1U; }
        }
    }
}
unsigned int can_channel_error_get(unsigned char channel)
{
    uint32_t peripheral;
    if (channel > 1U || !can_init_ok) { return CAN_ERROR_SOFTWARECFG; }
    peripheral = channel == 0U ? CAN0 : CAN1;
    if (can_flag_get(peripheral, CAN_FLAG_BOERR) != RESET ||
        can_flag_get(peripheral, CAN_FLAG_PERR) != RESET) {
        return CAN_ERROR_SOFTWARECFG;
    }
    return link_error[channel];
}
