#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>

// 回调函数，用于承载传入函数的指针
typedef void (* Serial_RxCallback)(uint8_t byte);

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_Printf(char *format, ...);

void Serial_RegisterRxCallback(Serial_RxCallback callback);

#endif
