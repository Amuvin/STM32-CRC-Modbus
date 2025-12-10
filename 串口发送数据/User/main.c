#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Serial.h"

uint8_t SendKeyNum = 1;	// 假设要发送的按键键码变量
float Sendfloat = 23.5; // 假设要发送的数据变量
int Sendint = -100;     // 假设要发送的整数数据变

uint8_t buffer[9]; // 定义一个字节数组用于存储数据帧

int main(void)
{
	/*模块初始化*/
	Serial_Init();		//串口初始化
	
	while (1)
	{
		// 打包数据
		memcpy(&buffer[0], &SendKeyNum, 1);         // 复制按键键码
		memcpy(&buffer[1], &Sendfloat, 4);          // 复制浮点数
		memcpy(&buffer[5], &Sendint, 4);            // 复制整数

        // 发送数据帧
        Serial_Send_Cmd_Data(0x01, buffer, sizeof(buffer)); // 使用命令字0x01发送数据帧

        Delay_ms(1000); // 延时1秒
	}
}
