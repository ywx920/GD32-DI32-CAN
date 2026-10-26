#ifndef __TASK_H__
#define __TASK_H__
#include "gd32f4xx.h"
#include "di_protocol.h"
extern di_config DI_Config;
extern unsigned int can0_state, can1_state;
extern unsigned int DI_ConfigLoaded, DI_StorageErrors;
extern uint32_t DI_ReportSkipped[2];
void DI_TaskInit(uint32_t now);
void DI_TaskPoll(uint32_t now);
void DI_ProcessCAN(unsigned int channel, const can_receive_message_struct *message);
void LED_TASK(void);
#endif
