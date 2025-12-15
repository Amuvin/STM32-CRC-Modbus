#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Serial.h"
#include "Serial_Protocol.h"

int main(void)
{
	Serial_Init();			//串口初始化
	Serial_Protocol_Init();	//串口协议初始化

	// 定义并初始化数据结构体
	DataStruct data = {
		.keyNum = 1,
		.floatData = 23.5,
		.intData = -100
	};
	
	while (1)
	{
		Serial_Protocol_SendStructData(0x01, &data); // 发送结构体数据，命令字为0x01

        Delay_ms(100); // 延时100ms
	}
}
