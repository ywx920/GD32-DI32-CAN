#ifdef DI_HOST_TEST
#include "tests/test_platform.h"
#else
#include "task.h"
#include "led.h"
#include "can.h"
#include "FNRAM.h"
#include "dim_input.h"
#include "systick.h"
#endif
#include <string.h>

di_config DI_Config;
unsigned int can0_state, can1_state;
unsigned int DI_ConfigLoaded, DI_StorageErrors;
uint32_t DI_ReportSkipped[2];
static unsigned int active_slot = 1U;
static uint32_t generation;
static uint32_t last_heart, last_status, last_led;
static uint8_t heart_counter;
/* One cached successful request permits retries to the old address/channel
   for 4 seconds after an address change. It never re-applies old settings. */
static unsigned int cache_valid;
static uint32_t cache_request_id, cache_response_id, cache_ms;
static uint8_t cache_request[8], cache_response[8];

static int config_equal(const di_config *a, const di_config *b)
{
    return a->address == b->address && a->channel == b->channel &&
           a->heartbeat_ms == b->heartbeat_ms && a->status_ms == b->status_ms;
}
static uint16_t slot_address(unsigned int slot)
{
    return (uint16_t)(0x0100U + slot * 0x0020U);
}
static void config_load(void)
{
    di_config candidate;
    uint32_t candidate_generation;
    uint8_t data[DI_CONFIG_BYTES];
    unsigned int slot;
    di_config_default(&DI_Config);
    for (slot = 0U; slot < 2U; ++slot) {
        if (FM_ReadStr(slot_address(slot), data, DI_CONFIG_BYTES) == 0U) {
            ++DI_StorageErrors; continue;
        }
        if (!di_config_decode(data, &candidate, &candidate_generation)) { continue; }
        if (!DI_ConfigLoaded ||
            ((candidate_generation - generation) != 0U &&
             (uint32_t)(candidate_generation - generation) < 0x80000000UL)) {
            DI_Config = candidate;
            generation = candidate_generation;
            active_slot = slot;
            DI_ConfigLoaded = 1U;
        }
    }
}
static int config_save(const di_config *candidate)
{
    uint8_t data[DI_CONFIG_BYTES], verify[DI_CONFIG_BYTES];
    uint8_t invalid[4] = {0U, 0U, 0U, 0U};
    unsigned int slot = active_slot ^ 1U;
    uint16_t address = slot_address(slot);
    if (DI_ConfigLoaded && config_equal(candidate, &DI_Config)) { return 1; }
    di_config_encode(candidate, generation + 1U, data);
    /* Invalidate inactive slot; body+CRC first, commit magic last.
       Previous valid slot remains available if power fails during writing. */
    if (FM_SendStr(address, invalid, 4U) == 0U ||
        FM_SendStr((uint16_t)(address + 4U), data + 4, DI_CONFIG_BYTES - 4U) == 0U ||
        FM_SendStr(address, data, 4U) == 0U ||
        FM_ReadStr(address, verify, DI_CONFIG_BYTES) == 0U ||
        memcmp(data, verify, DI_CONFIG_BYTES) != 0) {
        /* Best effort invalidation when a reported write/read error occurs. */
        (void)FM_SendStr(address, invalid, 4U);
        ++DI_StorageErrors;
        return 0;
    }
    active_slot = slot;
    ++generation;
    DI_ConfigLoaded = 1U;
    return 1;
}
static uint32_t report_id(uint8_t type)
{
    di_id id;
    id.type = type; id.source = DI_Config.address;
    id.destination = 0U; id.channel = DI_Config.channel;
    id.slave = 0U; id.sequence = 0U;
    return di_id_encode(&id);
}
static int report_both(uint8_t type, const uint8_t data[8])
{
    uint32_t id;
    unsigned int channel;
    int accepted = 0;
    /* Leave four queue positions for parameter replies. */
    id = report_id(type);
    for (channel = 0U; channel < 2U; ++channel) {
        if (can_tx_free(channel) > 4U) {
            accepted |= (int)send_can((uint8_t)channel, id, data, 8U);
        } else {
            ++DI_ReportSkipped[channel];
        }
    }
    return accepted;
}
void DI_TaskInit(uint32_t now)
{
    DI_ConfigLoaded = 0U;
    DI_StorageErrors = 0U;
    generation = 0U;
    active_slot = 1U;
    cache_valid = 0U;
    DI_ReportSkipped[0] = 0U;
    DI_ReportSkipped[1] = 0U;
    config_load();
    last_heart = now; last_status = now; last_led = now;
    heart_counter = 0U;
}
void DI_TaskPoll(uint32_t now)
{
    uint8_t data[8];
    uint32_t led_half_period;
    if (DI_Config.heartbeat_ms != 0U &&
        (uint32_t)(now - last_heart) >= DI_Config.heartbeat_ms) {
        memset(data, 0, sizeof(data));
        data[0] = heart_counter;
        if (report_both(DI_TYPE_HEARTBEAT, data)) {
            ++heart_counter;
        }
        last_heart = now;
    }
    if (DI_Config.status_ms != 0U &&
        (uint32_t)(now - last_status) >= DI_Config.status_ms) {
        di_status_pack(DIM_InputGetState(), data);
        (void)report_both(DI_TYPE_STATUS, data);
        last_status = now;
    }
    led_half_period = DI_Config.heartbeat_ms == 0U ? 500U :
                      ((uint32_t)DI_Config.heartbeat_ms + 1U) / 2U;
    if ((uint32_t)(now - last_led) >= led_half_period) {
        can0_state = can_channel_error_get(0U);
        can1_state = can_channel_error_get(1U);
        LED_TASK();
        last_led = now;
    }
}
void DI_ProcessCAN(unsigned int channel, const can_receive_message_struct *message)
{
    di_id id, reply;
    di_config candidate;
    uint8_t data[8];
    uint16_t value;
    uint32_t now, response_id;
    int success;
    if (channel > 1U || message == 0 || message->rx_ff != CAN_FF_EXTENDED ||
        message->rx_ft != CAN_FT_DATA || message->rx_dlen != 8U ||
        message->rx_efid > 0x1FFFFFFFU) { return; }
    di_id_decode(message->rx_efid, &id);
    if (id.type != DI_TYPE_PARAMETER || id.source == 0U ||
        id.slave != 0U || id.sequence != 0U || message->rx_data[3] != 0U) { return; }
    now = systick_millis();
    if (cache_valid && (uint32_t)(now - cache_ms) < 4000U &&
        message->rx_efid == cache_request_id &&
        memcmp(message->rx_data, cache_request, 8U) == 0) {
        (void)send_can((uint8_t)channel, cache_response_id, cache_response, 8U);
        return;
    }
    /* Parameter changes are unicast only; avoid broadcast writes and ACK storms. */
    if (id.destination != DI_Config.address || id.channel != DI_Config.channel ||
        id.source == DI_Config.address || can_tx_free(channel) == 0U) { return; }
    candidate = DI_Config;
    value = (uint16_t)(((uint16_t)message->rx_data[1] << 8) | message->rx_data[2]);
    success = di_parameter_update(&candidate, message->rx_data[0], value);
    if (success) { success = config_save(&candidate); }
    memcpy(data, message->rx_data, sizeof(data));
    data[3] = success ? 1U : 0U;
    reply = id;
    reply.source = DI_Config.address; reply.destination = id.source;
    /* Reply carries old address/channel, same sequence, and arrives on ingress bus. */
    response_id = di_id_encode(&reply);
    (void)send_can((uint8_t)channel, response_id, data, 8U);
    if (success) {
        cache_valid = 1U; cache_ms = now;
        cache_request_id = message->rx_efid; cache_response_id = response_id;
        memcpy(cache_request, message->rx_data, 8U);
        memcpy(cache_response, data, 8U);
        if (!config_equal(&DI_Config, &candidate)) {
            DI_Config = candidate;
            last_heart = now; last_status = now;
        }
    }
}

void LED_TASK(void)
{
    unsigned int fault_count = 0U;
    static unsigned int led_phase = 0U;

    led_phase ^= 1U;

    if (can0_state != CAN_ERROR_NONE) {
        fault_count++;
        LED_CAN1_CLR();
    } else if (led_phase != 0U) {
        LED_CAN1_SET();
    } else {
        LED_CAN1_CLR();
    }

    if (can1_state != CAN_ERROR_NONE) {
        fault_count++;
        LED_CAN2_CLR();
    } else if (led_phase != 0U) {
        LED_CAN2_SET();
    } else {
        LED_CAN2_CLR();
    }

    if (fault_count != 0U) {
        LED_FAU_SET();
        LED_RUN_CLR();
    } else {
        LED_FAU_CLR();
        if (led_phase != 0U) {
            LED_RUN_SET();
        } else {
            LED_RUN_CLR();
        }
    }
}

