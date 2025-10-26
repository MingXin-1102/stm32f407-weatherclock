#include "aht20.h"

#define AHT20_ADDRESS	0x70

#define I2C_CHECK_EVENT(EVENT, TIMEOUT)                     \
	do                                                      \
	{                                                       \
		uint32_t timeout = TIMEOUT;                         \
		while (!I2C_CheckEvent(I2C1, EVENT) && timeout > 0) \
		{                                                   \
			Delay_Us(10);                                   \
			timeout -= 10;                                  \
		}                                                   \
		if (timeout <= 10)                                  \
			return false;                                   \
	} while (0)

extern const float temperture,humidity;
extern const uint32_t failcount = 0;


static bool aht20_read_status(uint8_t *status);
static bool aht20_is_busy(void);
static bool aht20_is_ready(void);
static bool aht20_write(uint8_t data[], uint32_t length);
static bool aht20_read(uint8_t data[], uint32_t length);

bool aht20_init(void)
{
	/*SCL pin: PB6,SDA PB7*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; // I2C线开漏输出，必须上拉
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);

	I2C_InitTypeDef I2C_InitStruct;
	I2C_StructInit(&I2C_InitStruct);
	I2C_InitStruct.I2C_ClockSpeed = 100ul * 1000ul; // 标准100kHz
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C1, &I2C_InitStruct);
	I2C_Cmd(I2C1, ENABLE);

	Delay_Ms(40);
	if (aht20_is_ready())
		return true;

	if (!aht20_write((uint8_t []){0xBE,0x08,0x00}, 3))
		return false;

	for (uint32_t t = 0; t < 100; t++)
	{
		Delay_Us(1000);
		if (aht20_is_ready())
			return true;
	}
	return false;
}

static bool aht20_write(uint8_t data[], uint32_t length)
{
	/*AT24C02  地址0x38 补一位 0x70*/
	/*写操作*/
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	I2C_GenerateSTART(I2C1, ENABLE);
	I2C_CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT, 1000);
	I2C_Send7bitAddress(I2C1, AHT20_ADDRESS, I2C_Direction_Transmitter);
	I2C_CHECK_EVENT(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 1000);
	for (uint32_t i = 0; i < length; i++)
	{
		I2C_SendData(I2C1, data[i]);
		I2C_CHECK_EVENT(I2C_EVENT_MASTER_BYTE_TRANSMITTING, 1000);
	}
	I2C_GenerateSTOP(I2C1, ENABLE);

	return true;
}
static bool aht20_read(uint8_t data[], uint32_t length)
{
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	I2C_GenerateSTART(I2C1, ENABLE);
	I2C_CHECK_EVENT(I2C_EVENT_MASTER_MODE_SELECT, 1000);
	I2C_Send7bitAddress(I2C1, AHT20_ADDRESS, I2C_Direction_Receiver);
	I2C_CHECK_EVENT(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, 1000);
	for (uint32_t i = 0; i < length; i++)
	{
		if (i == length - 1)
			I2C_AcknowledgeConfig(I2C1,DISABLE);

		I2C_CHECK_EVENT(I2C_EVENT_MASTER_BYTE_RECEIVED, 1000); // 先等待再取数据
		data[i] = I2C_ReceiveData(I2C1);
	}
	I2C_GenerateSTOP(I2C1, ENABLE);

	return true;
}

static bool aht20_read_status(uint8_t *status)
{
	uint8_t cmd = 0x71;
	if (!aht20_write(&cmd, 1))
		return false;

	if (!aht20_read(status, 1))
		return false;

	return true;
}
static bool aht20_is_busy(void)
{
	uint8_t status;
	if (!aht20_read_status(&status))
		return false;

	//&上最高位
	return (status & 0x80) != 0;
}
static bool aht20_is_ready(void)
{
	uint8_t status;
	if (!aht20_read_status(&status))
		return false;

	//&上最高位
	return (status & 0x08) != 0;
}

bool aht20_start_measure(void)
{
	return aht20_write((uint8_t []){0xAC,0x33,0x00}, 3);
}

bool aht20_wait_for_measure(void)
{
	for (uint32_t t = 0; t < 200; t++)
	{
		Delay_Us(1000);
		if (!aht20_is_busy())
			return true;
	}
	return false;
}

bool aht20_read_measurement(float *temperture,float *humidity)
{
	uint8_t data[6];
	if (!aht20_read(data, 6))
		return false;
	uint32_t raw_humidity = ((uint32_t)(data[1] << 12)) |
							((uint32_t)(data[2] << 4)) |
							((uint32_t)(data[3] & 0x0F )>> 4);
	uint32_t raw_temperture = ((uint32_t)((data[3] & 0x0F) << 16)) |
							((uint32_t)(data[4] << 8)) |
							((uint32_t)(data[5] ));
	*humidity = (float)raw_humidity  * 100.0f / (float) 0x100000;
	*temperture = (float)raw_temperture  * 200.0f / (float) 0x100000 - 50.0f;

	return true ;
}



// int fputc(int ch,FILE *f)
// {
// 		USART_SendData(USART1, (uint8_t)ch);
// 		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
// 		return ch;
// }

