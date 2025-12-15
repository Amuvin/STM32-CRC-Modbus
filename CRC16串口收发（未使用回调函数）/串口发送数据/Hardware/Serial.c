#include "Serial.h"

// 定义一个全局变量来存储接收到的数据
static DataStruct data;

/**
  * 函    数：串口初始化
  * 参    数：无
  * 返 回 值：无
  */
void Serial_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//开启USART1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA9引脚初始化为复用推挽输出
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA10引脚初始化为上拉输入
	
	/*USART初始化*/
	USART_InitTypeDef USART_InitStructure;					//定义结构体变量
	USART_InitStructure.USART_BaudRate = 9600;				//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//硬件流控制，不需要
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//模式，发送模式和接收模式均选择
	USART_InitStructure.USART_Parity = USART_Parity_No;		//奇偶校验，不需要
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//停止位，选择1位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//字长，选择8位
	USART_Init(USART1, &USART_InitStructure);				//将结构体变量交给USART_Init，配置USART1
	
	/*中断输出配置*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);			//开启串口接收数据的中断
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//配置NVIC为分组2
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;					//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		//选择配置NVIC的USART1线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//指定NVIC线路的抢占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);							//将结构体变量交给NVIC_Init，配置NVIC外设
	
	/*USART使能*/
	USART_Cmd(USART1, ENABLE);								//使能USART1，串口开始运行
}

/**
  * 函    数：串口发送一个字节
  * 参    数：Byte 要发送的一个字节
  * 返 回 值：无
  */
void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);		//将字节数据写入数据寄存器，写入后USART自动生成时序波形
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	//等待发送完成
	/*下次写入数据寄存器会自动清除发送完成标志位，故此循环后，无需清除标志位*/
}

/**
  * 函    数：串口发送一个数组
  * 参    数：Array 要发送数组的首地址
  * 参    数：Length 要发送数组的长度
  * 返 回 值：无
  */
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		//遍历数组
	{
		Serial_SendByte(Array[i]);		//依次调用Serial_SendByte发送每个字节数据
	}
}

/**
  * 函    数：串口发送一个字符串
  * 参    数：String 要发送字符串的首地址
  * 返 回 值：无
  */
void Serial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//遍历字符数组（字符串），遇到字符串结束标志位后停止
	{
		Serial_SendByte(String[i]);		//依次调用Serial_SendByte发送每个字节数据
	}
}

/**
  * 函    数：使用printf需要重定向的底层函数
  * 参    数：保持原始格式即可，无需变动
  * 返 回 值：保持原始格式即可，无需变动
  */
int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);			//将printf的底层重定向到自己的发送字节函数
	return ch;
}

/**
  * 函    数：自己封装的prinf函数
  * 参    数：format 格式化字符串
  * 参    数：... 可变的参数列表
  * 返 回 值：无
  */
void Serial_Printf(char *format, ...)
{
	char String[100];				//定义字符数组
	va_list arg;					//定义可变参数列表数据类型的变量arg
	va_start(arg, format);			//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);	//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);					//结束变量arg
	Serial_SendString(String);		//串口发送字符数组（字符串）
}

/**
  * 函    数：计算CRC16/MODBUS校验码
  * 参    数：data 要校验的数据首地址
  * 参    数：len 要校验的数据长度
  * 返 回 值：返回CRC16校验值
  * 注意事项：多项式：x^16 + x^15 + x^2 + 1 (即 0x8005)
  * 		 初始值为：0xFFFF
  * 		 
  */
uint16_t Serial_CRC16_Check(const uint8_t *data,uint8_t len)
{
	uint16_t CRC16 = 0xFFFF;	 // 初始化CRC

	for (uint8_t i = 0; i < len; i ++)	// 遍历每一个输入数据字节
	{
		// 将当前数据字节与CRC寄存器的高8位进行异或
        // 这一步相当于把新的8位数据“喂”给CRC计算引擎，整合之前所有数据的CRC值
		CRC16 ^= (data[i] << 8);

		// 由于数据data的位数为8位，需要计算8次
		for (uint8_t bit = 0; bit < 8; bit ++)
		{
			if (CRC16 & 0x8000) // 检查数据最高位是否为1
			{
				CRC16 <<= 1;	// 如果最高位是1，CRC寄存器先左移一位 (丢弃最高位)
								// 由于多项式生成的除数为1 1000 0000 0000 0101为17位
								// 而程序除数1000 0000 0000 0101只有16位，相比于理论除数少了一位
								// 所以得从被除数补回来，抵消这个少了一位除数的影响

				CRC16 ^= 0x8005;// 然后进行异或操作，算出CRC值
			} 
			else 
			{
				CRC16 <<= 1;	// 没有找到首位为1，继续移动寻找
			}
		}
	}

	return CRC16;
}

/**
  * 函    数：发送数据帧
  * 参    数：cmd 命令字节
  * 参    数：datas 数据数组首地址
  * 参    数：len 总字节长度
  * 返 回 值：无
  */
void Serial_Send_Cmd_Data(uint8_t cmd,const uint8_t *datas,uint8_t len)
{
    uint8_t buf[300],i,cnt=0;
    uint16_t crc16;
    buf[cnt++] = 0xA5;
    buf[cnt++] = 0x5A;
    buf[cnt++] = len;
    buf[cnt++] = cmd;
    for(i=0;i<len;i++)
    {
        buf[cnt++] = datas[i];
    }
    crc16 = Serial_CRC16_Check(buf,len+4);
    buf[cnt++] = crc16>>8;
    buf[cnt++] = crc16&0xFF;
    buf[cnt++] = 0xFF;
    Serial_SendArray(buf,cnt);// 调用数据帧发送函数将打包好的数据帧发送出去
}

/**
  * 函    数：发送结构体
  * 参    数：cmd 命令
  * 参    数：data 指向要发送的DataStruct结构体的指针
  * 返 回 值：无
  * 注意事项：此函数会将结构体格式化为字符串，并打包成数据帧发送
  */
void Serial_SendStructData(uint8_t cmd, DataStruct *data)
{
	uint8_t buffer[300];
	int len = snprintf((char *)buffer, sizeof(buffer), "keyNum:%d,floatData:%4.2f,intData:%d",
					  data->keyNum, data->floatData, data->intData);

	Serial_Send_Cmd_Data(cmd, buffer, len); // 使用指定命令字发送数据帧
}

/**
  * 函    数：根据命令解析函数
  * 参    数：cmd 命令字节
  * 参    数：datas 数据数组首地址
  * 参    数：len 总字节长度
  * 返 回 值：无
  * 注意事项：根据需要处理数据
  */
void Serial_DataAnalysis(uint8_t cmd,const uint8_t *datas,uint8_t len)
{
	switch(cmd)
	{
		case 0x01:
			sscanf((const char *)datas, "keyNum:%hhu,floatData:%f,intData:%d",
				   &data.keyNum,
				   &data.floatData,
				   &data.intData);
			break;
		case 0x02:

			break;
		default:
			break;
	}
}

/**
  * 函    数：返回接收到的数据结构体
  * 参    数：无
  * 返 回 值：返回接收到的数据结构体
  */
DataStruct Serial_GetReceivedStructData(void)
{
	return data;
}

/**
  * 函    数：串口接收状态机
  * 参    数：bytedata 接收到的一个字节数据
  * 返 回 值：无
  * 注意事项：此函数为状态机，接收数据后自动调用
  */
void Serial_Receive(uint8_t bytedata)
{
	static uint8_t state = 0;	// 状态变量初始化为0 在函数中必须为静态变量
	static uint8_t cnt = 0;  	// 计算元素的个数
	static uint8_t cmd, *data_ptr;	// 存储命令,存储数据的指针
	static uint8_t Buf[300], len;	// 储存接收的信号，数据的长度，用于CRC校验
	static uint16_t crc16;		// 存储CRC校验值
	
	switch (state)
	{
	case 0:
		if (bytedata == 0xA5)
		{
			cnt = 0;
			Buf[cnt++] = bytedata;
			state = 1; // 切换到另一个包头状态
		}
		break;

	case 1:
		if (bytedata == 0x5A)
		{
			Buf[cnt++] = bytedata;
			state = 2; // 切换到长度接收状态
		}
		else
		{
			state = 0; // 如果不是包头，回到初始状态
		}
		break;

	case 2:
		Buf[cnt++] = bytedata;
		len = bytedata;
		state = 3; // 切换到命令接收状态
		break;

	case 3:
		Buf[cnt++] = bytedata;
		cmd = bytedata;
		data_ptr = &Buf[cnt];// 记录数据指针首地址
		state = 4; // 切换到数据接收状态
		if (len == 0) state = 5;// 数据字节长度为0则跳过数据接收状态
		break;
	
	case 4:
		Buf[cnt++] = bytedata;
		if (data_ptr + len == &Buf[cnt]) // 检查是否接收完所有数据字节
		{
			state = 5; // 切换到CRC接收状态
		}
		break;

	case 5:
		crc16 = bytedata << 8; // 高字节
		state = 6; // 切换到CRC低字节接收状态
		break;

	case 6:
		crc16 |= bytedata; // 低字节
		if (crc16 == Serial_CRC16_Check(Buf, len + 4)) // 校验CRC
		{
			state = 7; // 切换到数据帧接收完成状态
		}
		else
		{
			state = 0; // CRC校验失败，回到初始状态
		}
		break;

	case 7:
		if (bytedata == 0xFF)
		{
			Serial_DataAnalysis(cmd, data_ptr, len);	// 调用数据解析函数处理接收到的数据
		}
		state = 0; // 无论成功与否，回到初始状态准备接收下一个数据帧
		break;
	default:
		state = 0;
		break;
	}
}

/**
  * 函    数：USART1中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)		//判断是否是USART1的接收事件触发的中断
	{
		static uint8_t RxData;
        RxData = USART_ReceiveData(USART1);

		Serial_Receive(RxData);		//调用接收数据解析函数，传入接收到的字节数据
		
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);		//清除标志位
	}
}
