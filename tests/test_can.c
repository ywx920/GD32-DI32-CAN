#include "can.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
unsigned int ICAN_LED1, ICAN_LED2;
static can_transmit_state_enum state[2] = {CAN_TRANSMIT_PENDING, CAN_TRANSMIT_PENDING};
static can_transmit_message_struct captured[2];
static unsigned int sends[2], aborts[2];
static FlagStatus bus_off;
void rcu_periph_clock_enable(rcu_periph_enum p) { (void)p; }
uint32_t rcu_clock_freq_get(rcu_clock_freq_enum c) { (void)c; return 50000000U; }
void gpio_mode_set(uint32_t p,uint32_t m,uint32_t u,uint32_t n) { (void)p;(void)m;(void)u;(void)n; }
void gpio_output_options_set(uint32_t p,uint8_t t,uint32_t s,uint32_t n) { (void)p;(void)t;(void)s;(void)n; }
void gpio_af_set(uint32_t p,uint32_t a,uint32_t n) { (void)p;(void)a;(void)n; }
void can_deinit(uint32_t p) { (void)p; }
ErrStatus can_init(uint32_t p,can_parameter_struct *c)
{
    (void)p; assert(c->prescaler==20U && c->auto_bus_off_recovery==ENABLE);
    return SUCCESS;
}
void can_filter_init(can_filter_parameter_struct *f)
{
    assert(f->filter_list_low==4U && f->filter_mask_low==6U);
}
void can_interrupt_enable(uint32_t p,uint32_t i) { (void)p;(void)i; }
void nvic_irq_enable(IRQn_Type i,uint8_t p,uint8_t s) { (void)i;(void)p;(void)s; }
uint8_t can_message_transmit(uint32_t p,can_transmit_message_struct *m)
{
    unsigned int ch=p==CAN0?0U:1U;
    captured[ch]=*m; ++sends[ch]; state[ch]=CAN_TRANSMIT_PENDING;
    return CAN_MAILBOX0;
}
can_transmit_state_enum can_transmit_states(uint32_t p,uint8_t m) { (void)m; return state[p==CAN0?0U:1U]; }
ErrStatus can_transmission_stop(uint32_t p,uint8_t m)
{
    (void)m; ++aborts[p==CAN0?0U:1U]; return SUCCESS;
}
FlagStatus can_flag_get(uint32_t p,can_flag_enum f) { (void)p; return f==CAN_FLAG_BOERR?bus_off:RESET; }
int main(void)
{
    uint8_t data[8]={0x80U,0U,0U,1U,0U,0U,0U,0U};
    unsigned int i;
    MY_CAN_INIT(); assert(can_init_ok);
    assert(can_tx_free(0U)==15U);
    assert(!send_can(2U,1U,data,8U)); assert(!send_can(0U,0x20000000U,data,8U));
    assert(!send_can(0U,1U,data,9U)); assert(!send_can(0U,1U,0,8U));
    for(i=0U;i<15U;++i) assert(send_can(0U,0x02581B00U+i,data,8U));
    assert(can_tx_free(0U)==0U && !send_can(0U,1U,data,8U));
    can_tx_poll(0U); assert(sends[0]==0U);
    can_tx_poll(1U); assert(sends[0]==1U && sends[1]==0U);
    assert(captured[0].tx_ff==CAN_FF_EXTENDED && captured[0].tx_ft==CAN_FT_DATA);
    assert(captured[0].tx_efid==0x02581B00U && captured[0].tx_dlen==8U);
    assert(memcmp(captured[0].tx_data,data,8U)==0);
    can_tx_poll(2U); assert(sends[0]==1U);
    state[0]=CAN_TRANSMIT_OK; can_tx_poll(2U); assert(can_tx_free(0U)==1U);
    can_tx_poll(3U); assert(sends[0]==2U && captured[0].tx_efid==0x02581B01U);
    state[0]=CAN_TRANSMIT_FAILED; can_tx_poll(4U); assert(can_tx_failed[0]==1U);
    can_tx_poll(13U); assert(sends[0]==2U);
    can_tx_poll(14U); assert(sends[0]==3U);
    state[0]=CAN_TRANSMIT_FAILED; can_tx_poll(15U);
    can_tx_poll(25U); assert(sends[0]==4U);
    state[0]=CAN_TRANSMIT_FAILED; can_tx_poll(26U);
    assert(can_tx_free(0U)==2U && can_tx_dropped[0]==2U);
    assert(can_channel_error_get(0U)!=CAN_ERROR_NONE);
    assert(send_can(1U,0x02581B00U,data,8U));
    can_tx_poll(100U); assert(sends[1]==1U);
    state[0]=CAN_TRANSMIT_OK;
    can_tx_poll(200U); assert(aborts[1]==1U && can_tx_timeouts[1]==1U);
    can_tx_poll(210U); assert(sends[1]==2U);
    state[1]=CAN_TRANSMIT_OK; can_tx_poll(211U);
    assert(can_tx_free(1U)==15U && can_channel_error_get(1U)==CAN_ERROR_NONE);
    bus_off=SET; assert(can_channel_error_get(1U)!=CAN_ERROR_NONE);
    puts("PASS: CAN bit rate/filter/extended payload/queue bounds/1 ms spacing/retry limit/timeout/dual bus isolation");
    return 0;
}
