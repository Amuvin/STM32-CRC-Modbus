#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Serial.h"

// 定义一个结构体来存储数据
typedef struct {
    uint8_t keyNum;     // 按键键码
    float floatData;    // 浮点数数据
    int intData;        // 整数数据
} DataStruct;

uint8_t buffer[300]; // 定义一个字节数组用于存储数据帧

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
		int len = snprintf(buffer, sizeof(buffer), "keyNum:%d,floatData:%3.2f,intData:%d",
				data.keyNum, data.floatData, data.intData);

        // 发送数据帧
        Serial_Send_Cmd_Data(0x01, buffer, len); // 使用命令字0x01发送数据帧

        Delay_ms(1000); // 延时1秒
	}
}
