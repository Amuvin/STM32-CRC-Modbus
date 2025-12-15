#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>

// 定义一个结构体来存储数据
typedef struct {
    uint8_t keyNum;     // 按键键码
    float floatData;    // 浮点数数据
    int intData;        // 整数数据
} DataStruct;

void Serial_Init(void);
void Serial_Printf(char *format, ...);

void Serial_SendStructData(uint8_t cmd, DataStruct *data);

#endif
