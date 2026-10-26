#include "dim_input.h"

static uint32_t dim_input_state;

#define DIM_GPIOA_PINS (GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_15)
#define DIM_GPIOB_PINS (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4 | \
                        GPIO_PIN_5 | GPIO_PIN_14 | GPIO_PIN_15)
#define DIM_GPIOC_PINS (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_9 | GPIO_PIN_10 | \
                        GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | \
                        GPIO_PIN_15)
#define DIM_GPIOD_PINS (GPIO_PIN_2 | GPIO_PIN_3)
#define DIM_GPIOE_PINS (GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6)
#define DIM_GPIOG_PINS (GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14)
#define DIM_SIN_OE_PORT GPIOC
#define DIM_SIN_OE_PIN  GPIO_PIN_6

static uint32_t dim_active_low_bit(uint32_t port, uint32_t pin, uint32_t bit)
{
    return (gpio_input_bit_get(port, pin) == RESET) ? (1UL << bit) : 0UL;
}

void DIM_InputInit(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOE);
    rcu_periph_clock_enable(RCU_GPIOG);

    /* Match the delivered firmware: all S_IN signals are active-low inputs
       configured with the MCU's internal pull-down. */
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, DIM_GPIOA_PINS);
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, DIM_GPIOB_PINS);
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, DIM_GPIOC_PINS);
    gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, DIM_GPIOD_PINS);
    gpio_mode_set(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, DIM_GPIOE_PINS);
    gpio_mode_set(GPIOG, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, DIM_GPIOG_PINS);

    /* J4-38 SIN_OE -> CT2-38 UART1_TXD -> PC6, active low. */
    gpio_bit_reset(DIM_SIN_OE_PORT, DIM_SIN_OE_PIN);
    gpio_mode_set(DIM_SIN_OE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                  DIM_SIN_OE_PIN);
    gpio_output_options_set(DIM_SIN_OE_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            DIM_SIN_OE_PIN);

    DIM_InputSample();
}

void DIM_InputSample(void)
{
    uint32_t state = 0UL;

    state |= dim_active_low_bit(GPIOC, GPIO_PIN_9, 0U);
    state |= dim_active_low_bit(GPIOA, GPIO_PIN_8, 1U);
    state |= dim_active_low_bit(GPIOE, GPIO_PIN_2, 2U);
    state |= dim_active_low_bit(GPIOE, GPIO_PIN_3, 3U);

    state |= dim_active_low_bit(GPIOC, GPIO_PIN_10, 4U);
    state |= dim_active_low_bit(GPIOC, GPIO_PIN_11, 5U);
    state |= dim_active_low_bit(GPIOE, GPIO_PIN_4, 6U);
    state |= dim_active_low_bit(GPIOE, GPIO_PIN_5, 7U);
    state |= dim_active_low_bit(GPIOC, GPIO_PIN_12, 8U);
    state |= dim_active_low_bit(GPIOD, GPIO_PIN_2, 9U);
    state |= dim_active_low_bit(GPIOE, GPIO_PIN_6, 10U);
    state |= dim_active_low_bit(GPIOC, GPIO_PIN_13, 11U);
    state |= dim_active_low_bit(GPIOD, GPIO_PIN_3, 12U);
    state |= dim_active_low_bit(GPIOG, GPIO_PIN_9, 13U);
    state |= dim_active_low_bit(GPIOC, GPIO_PIN_14, 14U);
    state |= dim_active_low_bit(GPIOC, GPIO_PIN_15, 15U);
    state |= dim_active_low_bit(GPIOG, GPIO_PIN_11, 16U);
    state |= dim_active_low_bit(GPIOG, GPIO_PIN_12, 17U);
    state |= dim_active_low_bit(GPIOA, GPIO_PIN_6, 18U);
    state |= dim_active_low_bit(GPIOA, GPIO_PIN_7, 19U);
    state |= dim_active_low_bit(GPIOG, GPIO_PIN_13, 20U);
    state |= dim_active_low_bit(GPIOG, GPIO_PIN_14, 21U);
    state |= dim_active_low_bit(GPIOC, GPIO_PIN_4, 22U);
    state |= dim_active_low_bit(GPIOC, GPIO_PIN_5, 23U);
    state |= dim_active_low_bit(GPIOB, GPIO_PIN_3, 24U);
    state |= dim_active_low_bit(GPIOB, GPIO_PIN_4, 25U);
    state |= dim_active_low_bit(GPIOB, GPIO_PIN_0, 26U);
    state |= dim_active_low_bit(GPIOB, GPIO_PIN_1, 27U);
    state |= dim_active_low_bit(GPIOB, GPIO_PIN_5, 28U);
    state |= dim_active_low_bit(GPIOA, GPIO_PIN_15, 29U);
    state |= dim_active_low_bit(GPIOB, GPIO_PIN_15, 30U);
    state |= dim_active_low_bit(GPIOB, GPIO_PIN_14, 31U);

    dim_input_state = state;
}

uint32_t DIM_InputGetState(void)
{
    return dim_input_state;
}
