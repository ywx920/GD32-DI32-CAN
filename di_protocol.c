#include "di_protocol.h"
#include <string.h>
static uint16_t get16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}
static void put16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8); p[1] = (uint8_t)value;
}
static uint16_t crc16(const uint8_t *p, unsigned int length)
{
    uint16_t crc = 0xFFFFU;
    unsigned int i, bit;
    for (i = 0U; i < length; ++i) {
        crc ^= (uint16_t)((uint16_t)p[i] << 8);
        for (bit = 0U; bit < 8U; ++bit) {
            crc = (uint16_t)((crc & 0x8000U) ?
                  ((uint32_t)crc << 1) ^ 0x1021U : ((uint32_t)crc << 1));
        }
    }
    return crc;
}
uint32_t di_id_encode(const di_id *id)
{
    return ((uint32_t)(id->type & 31U) << 24) |
           ((uint32_t)(id->source & 31U) << 19) |
           ((uint32_t)(id->destination & 31U) << 14) |
           ((uint32_t)(id->channel & 63U) << 8) |
           ((uint32_t)(id->slave & 15U) << 4) | (id->sequence & 15U);
}
void di_id_decode(uint32_t raw, di_id *id)
{
    id->type = (uint8_t)((raw >> 24) & 31U);
    id->source = (uint8_t)((raw >> 19) & 31U);
    id->destination = (uint8_t)((raw >> 14) & 31U);
    id->channel = (uint8_t)((raw >> 8) & 63U);
    id->slave = (uint8_t)((raw >> 4) & 15U);
    id->sequence = (uint8_t)(raw & 15U);
}
void di_config_default(di_config *config)
{
    config->address = DI_DEFAULT_ADDRESS;
    config->channel = DI_DEFAULT_CHANNEL;
    config->heartbeat_ms = DI_DEFAULT_PERIOD_MS;
    config->status_ms = DI_DEFAULT_PERIOD_MS;
}
int di_config_valid(const di_config *config)
{
    return config->address >= 1U && config->address <= 31U &&
           config->channel >= 1U && config->channel <= 63U;
}
void di_status_pack(uint32_t state, uint8_t data[8])
{
    memset(data, 0, 8U);
    data[0] = (uint8_t)(state >> 24); data[1] = (uint8_t)(state >> 16);
    data[2] = (uint8_t)(state >> 8); data[3] = (uint8_t)state;
}
int di_parameter_update(di_config *config, uint8_t parameter, uint16_t value)
{
    switch (parameter) {
    case 1U:
        /* Fixed 250 kbit/s network; other speed encodings unspecified. */
        return value == DI_CAN_KBIT;
    case 2U:
        if (value < 1U || value > 63U) { return 0; }
        config->channel = (uint8_t)value; return 1;
    case 3U:
        if (value < 1U || value > 31U) { return 0; }
        config->address = (uint8_t)value; return 1;
    case 4U: config->heartbeat_ms = value; return 1;
    case 5U: config->status_ms = value; return 1;
    default: return 0;
    }
}
void di_config_encode(const di_config *config, uint32_t generation,
                      uint8_t data[DI_CONFIG_BYTES])
{
    memset(data, 0, DI_CONFIG_BYTES);
    memcpy(data, "DI14", 4U); data[4] = 1U;
    data[5] = config->address; data[6] = config->channel;
    put16(data + 8, DI_CAN_KBIT);
    put16(data + 10, config->heartbeat_ms);
    put16(data + 12, config->status_ms);
    data[14] = (uint8_t)(generation >> 24);
    data[15] = (uint8_t)(generation >> 16);
    data[16] = (uint8_t)(generation >> 8);
    data[17] = (uint8_t)generation;
    put16(data + 18, crc16(data, 18U));
}
int di_config_decode(const uint8_t data[DI_CONFIG_BYTES], di_config *config,
                     uint32_t *generation)
{
    di_config candidate;
    if (memcmp(data, "DI14", 4U) != 0 || data[4] != 1U ||
        get16(data + 8) != DI_CAN_KBIT ||
        get16(data + 18) != crc16(data, 18U)) { return 0; }
    candidate.address = data[5]; candidate.channel = data[6];
    candidate.heartbeat_ms = get16(data + 10);
    candidate.status_ms = get16(data + 12);
    if (!di_config_valid(&candidate)) { return 0; }
    *config = candidate;
    *generation = ((uint32_t)data[14] << 24) |
                  ((uint32_t)data[15] << 16) |
                  ((uint32_t)data[16] << 8) | data[17];
    return 1;
}

