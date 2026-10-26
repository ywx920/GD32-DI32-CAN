#ifndef DI_PROTOCOL_H
#define DI_PROTOCOL_H
#include <stdint.h>
#define DI_TYPE_STATUS 2U
#define DI_TYPE_HEARTBEAT 4U
#define DI_TYPE_PARAMETER 7U
// 箱地址从11开始，到17
#define DI_DEFAULT_ADDRESS 17U //改
// 一个箱里2块DI板卡，分别为27，28
#define DI_DEFAULT_CHANNEL 28U //改
#define DI_DEFAULT_PERIOD_MS 2000U 
#define DI_CAN_KBIT 250U
#define DI_CONFIG_BYTES 20U
typedef struct {
    uint8_t address, channel;
    uint16_t heartbeat_ms, status_ms;
} di_config;
typedef struct {
    uint8_t type, source, destination, channel, slave, sequence;
} di_id;
uint32_t di_id_encode(const di_id *id);
void di_id_decode(uint32_t raw, di_id *id);
void di_config_default(di_config *config);
int di_config_valid(const di_config *config);
void di_status_pack(uint32_t state, uint8_t data[8]);
int di_parameter_update(di_config *config, uint8_t parameter, uint16_t value);
void di_config_encode(const di_config *config, uint32_t generation,
                      uint8_t data[DI_CONFIG_BYTES]);
int di_config_decode(const uint8_t data[DI_CONFIG_BYTES], di_config *config,
                     uint32_t *generation);
#endif
