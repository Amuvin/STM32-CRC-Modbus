#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Serial.h"
#include "OLED.h"

int main(void)
{
	/*模块初始化*/
	Serial_Init();		//串口初始化
	OLED_Init();		//OLED初始化

	// 定义并初始化数据结构体
	DataStruct data;
	
	while (1)
	{
		data = Serial_GetReceivedStructData(); // 获取接收到的数据结构体

		OLED_Printf(0, 0, OLED_8X16, "Key: %d", data.keyNum);
		OLED_Printf(0, 16, OLED_8X16, "Float: %.2f", data.floatData);
		OLED_Printf(0, 32, OLED_8X16, "Int: %d", data.intData);
		OLED_Update(); // 更新OLED显示内容

        Delay_ms(100); // 延时100毫秒
	}
}
