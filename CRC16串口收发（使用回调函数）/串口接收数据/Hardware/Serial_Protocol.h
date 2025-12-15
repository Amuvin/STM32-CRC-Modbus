#ifndef __SERIAL_PROTOCOL_H__
#define __SERIAL_PROTOCOL_H__

#include "stm32f10x.h"

// 定义一个结构体来存储数据
typedef struct {
    uint8_t keyNum;     // 按键键码
    float floatData;    // 浮点数数据
    int intData;        // 整数数据
} DataStruct;

void Serial_Protocol_Init(void);

void Serial_Protocol_SendStructData(uint8_t cmd, DataStruct *data);
DataStruct Serial_Protocol_GetReceivedStructData(void);

#endif
