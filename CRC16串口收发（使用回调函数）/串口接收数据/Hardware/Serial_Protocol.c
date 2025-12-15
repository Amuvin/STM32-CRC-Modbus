/**
 ******************************************************************************
 * @file    Serial_Protocol.c
 * @author  Amuvin
 * @version 1.0.0
 * @date    2025年12月15日
 * @brief   串口通信协议实现文件
 *          本文件实现了基于回调函数的串口通信协议，包括数据发送、
 *          接收状态机、CRC校验等功能。
 ******************************************************************************
 * @attention
 * 本代码仅供学习和参考使用，未经授权不得用于商业用途。
 *
 ******************************************************************************
 * @使用方法
 * 1. 调用 `Serial_Protocol_Init()` 初始化串口协议，并注册接收回调函数。
 * 2. 使用 `Serial_Protocol_SendStructData()` 发送数据结构体。
 * 3. 在接收到数据帧后，状态机会自动调用 `Serial_Protocol_Receive()` 处理数据。
 * 4. 可通过 `Serial_Protocol_GetReceivedStructData()` 获取解析后的数据结构体。
 *
 * @依赖
 * - 本模块依赖于 Serial 模块，用于底层串口数据的发送和接收。
 * - 需要确保 Serial 模块已正确初始化。
 * 
 * @注意事项
 * - 如果需要修改接收数据的解析，请修改 `Serial_Protocol_DataAnalysis()` 函数和结构体DataStruct定义。
 * - 如果需要修改发送数据的格式，请修改 `Serial_Protocol_SendStructData()` 函数和结构体DataStruct定义。
 *
 ******************************************************************************
 */
#include "Serial_Protocol.h"
#include "Serial.h"

static DataStruct data; // 用于存储接收到的数据结构体

static void Serial_Protocol_Receive(uint8_t bytedata); // 添加静态函数前向声明

/**
  * 函    数：串口协议初始化，注册接收回调函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：将Serial_Protocol_Receive函数作为参数传入到Serial文件
  * 		 调用此函数后，串口接收数据时会自动调用Serial_Protocol_Receive函数进行数据处理
  */
void Serial_Protocol_Init(void)
{
	Serial_RegisterRxCallback(Serial_Protocol_Receive); // 注册接收回调函数
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
static uint16_t Serial_Protocol_CRC16Check(const uint8_t *data,uint8_t len)
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
static void Serial_Protocol_SendCmdData(uint8_t cmd,const uint8_t *datas,uint8_t len)
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
    crc16 = Serial_Protocol_CRC16Check(buf,len+4);
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
void Serial_Protocol_SendStructData(uint8_t cmd, DataStruct *data)
{
	uint8_t buffer[300];
	int len = snprintf((char *)buffer, sizeof(buffer), "keyNum:%d,floatData:%4.2f,intData:%d",
					  data->keyNum, data->floatData, data->intData);

	Serial_Protocol_SendCmdData(cmd, buffer, len); // 使用指定命令字发送数据帧
}

/**
  * 函    数：根据命令解析函数
  * 参    数：cmd 命令字节
  * 参    数：datas 数据数组首地址
  * 参    数：len 总字节长度
  * 返 回 值：无
  * 注意事项：根据需要处理数据
  */
static void Serial_Protocol_DataAnalysis(uint8_t cmd,const uint8_t *datas,uint8_t len)
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
DataStruct Serial_Protocol_GetReceivedStructData(void)
{
	return data;
}

/**
  * 函    数：串口接收状态机
  * 参    数：bytedata 接收到的一个字节数据
  * 返 回 值：无
  * 注意事项：此函数为状态机，接收数据后自动调用
  */
static void Serial_Protocol_Receive(uint8_t bytedata)
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
		if (crc16 == Serial_Protocol_CRC16Check(Buf, len + 4)) // 校验CRC
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
			Serial_Protocol_DataAnalysis(cmd, data_ptr, len);	// 调用数据解析函数处理接收到的数据
		}
		state = 0; // 无论成功与否，回到初始状态准备接收下一个数据帧
		break;
	default:
		state = 0;
		break;
	}
}
