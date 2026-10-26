#ifndef __DIM_INPUT_H__
#define __DIM_INPUT_H__

#include "gd32f4xx.h"

void DIM_InputInit(void);
void DIM_InputSample(void);
uint32_t DIM_InputGetState(void);

#endif
