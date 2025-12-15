#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Serial.h"

int main(void)
{
	/*模块初始化*/
	Serial_Init();		//串口初始化

	// 定义并初始化数据结构体
	DataStruct data = {
		.keyNum = 1,
		.floatData = 23.5,
		.intData = -100
	};
	
	while (1)
	{
		Serial_SendStructData(0x01, &data); // 发送结构体数据，命令字为0x01

        Delay_ms(1000); // 延时1秒
	}
}
