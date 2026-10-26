#ifndef TEST_PLATFORM_H
#define TEST_PLATFORM_H
#include "../di_protocol.h"
#include <stdint.h>
#define CAN_FF_EXTENDED 1U
#define CAN_FF_STANDARD 0U
#define CAN_FT_DATA 0U
#define CAN_ERROR_NONE 0U
typedef struct {
    uint32_t rx_sfid, rx_efid;
    uint8_t rx_ff, rx_ft, rx_dlen, rx_data[8];
} can_receive_message_struct;
extern di_config DI_Config;
extern unsigned int DI_ConfigLoaded, DI_StorageErrors;
void DI_TaskInit(uint32_t now);
void DI_TaskPoll(uint32_t now);
void DI_ProcessCAN(unsigned int channel, const can_receive_message_struct *message);
void LED_TASK(void);
uint8_t FM_SendStr(uint16_t address, uint8_t *data, uint8_t length);
uint8_t FM_ReadStr(uint16_t address, uint8_t *data, uint8_t length);
uint32_t DIM_InputGetState(void);
uint32_t systick_millis(void);
unsigned int send_can(unsigned char ch, unsigned int id, const unsigned char *data, unsigned char len);
unsigned int can_tx_free(unsigned int ch);
unsigned int can_channel_error_get(unsigned char ch);
#define LED_CAN1_SET() ((void)0)
#define LED_CAN1_CLR() ((void)0)
#define LED_CAN2_SET() ((void)0)
#define LED_CAN2_CLR() ((void)0)
#define LED_RUN_SET() ((void)0)
#define LED_RUN_CLR() ((void)0)
#define LED_FAU_SET() ((void)0)
#define LED_FAU_CLR() ((void)0)
#endif
