#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_Printf(char *format, ...);

uint16_t Serial_CRC16_Check(const uint8_t *data,uint8_t len);
void Serial_Send_Cmd_Data(uint8_t cmd,const uint8_t *datas,uint8_t len);
void Serial_DataAnalysis(uint8_t cmd,const uint8_t *datas,uint8_t len);

#endif
