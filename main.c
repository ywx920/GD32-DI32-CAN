#include "gd32f4xx.h"
#include "systick.h"
#include "led.h"
#include "can.h"
#include "irq.h"
#include "task.h"
#include "FNRAM.h"
#include "dim_input.h"
int main(void)
{
    unsigned int channel, budget;
    can_receive_message_struct message;
    MY_LED_INIT();
    FNRAM_init();
    DIM_InputInit();
    systick_config();
    DI_TaskInit(systick_millis());
    MY_CAN_INIT();
    while (1) {
        DIM_InputSample();
        can_tx_poll(systick_millis());
        for (channel = 0U; channel < 2U; ++channel) {
            /* Bound processing so a busy bus cannot starve periodic work. */
            for (budget = 0U; budget < 8U; ++budget) {
                if (!can_rx_pop(channel, &message)) { break; }
                DI_ProcessCAN(channel, &message);
            }
        }
        DI_TaskPoll(systick_millis());
    }
}
