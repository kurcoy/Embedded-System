/* This file is the driver for RTC_DS3232 */
#include "multi-task.h"
#include "RTC3232_task.h"
#include "stm32f4xx_spi.h"
#include "debug.h"
#include "stm32f4xx.h"
#include "task4.h"
Time_Typedef TimeValue;	
uint8_t Time_Buffer[7];	//Ê±¼äÈÕÀúÊý¾Ý»º´
static char RtcDebugBuffer[128];
static DS3232_RegTable_t DS3232_RegTable[] =
{
		{0x00,	0x0},//second
		{0x01,	0x0},//minute
		{0x02,	0x0},//hour
		{0x03,	0x04},//day
		{0x04,	0x17},//date
		{0x05,	0x03},//month
		{0x06,	0x11},//year 2017 03 23
};

/**
  * @brief  Converts a 2 digit decimal to BCD format.
  * @param  Value: Byte to be converted.
  * @retval Converted byte
  */
static uint8_t RTC_ByteToBcd2(uint8_t Value)
{
  uint8_t bcdhigh = 0;
  
  while (Value >= 10)
  {
    bcdhigh++;
    Value -= 10;
  }
  
  return  ((uint8_t)(bcdhigh << 4) | Value);
}

/**
  * @brief  Convert from 2 digit BCD to Binary.
  * @param  Value: BCD value to be converted.
  * @retval Converted word
  */
static uint8_t RTC_Bcd2ToByte(uint8_t Value)
{
  uint8_t tmp = 0;
  tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
  return (tmp + (Value & (uint8_t)0x0F));
}

#if 0
static uint32_t I2C_Transmit(I2C_TypeDef * bus, I2C_TransmiData_t data)
{
	uint32_t ret = 0;
	uint32_t timeout = I2C_TIMEOUT_MAX;

	/* step 1 -> generate the start bit */
	I2C_GenerateSTART(bus, ENABLE);

	while(!I2C_CheckEvent(bus, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if(!(timeout--))
		{
			ret = 1;
			goto error;
		}
	}

	switch(data.dir)
	{
	default:
	case I2C_ReadOnly:
		if(data.read == NULL)
		{
			goto error;
		}
		/* step 2 -> send the slave address+r */
		timeout = I2C_TIMEOUT_MAX;
		I2C_Send7bitAddress(bus, (uint8_t)((data.slave << 1) | 0x01), I2C_Direction_Receiver);

		while(!I2C_CheckEvent(bus, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
		{
			if(!(timeout--))
			{
				ret = 2;
				goto error;
			}
		}
		/* step 3 -> to receive the data */
		while(data.rLen--)
		{
			if(data.rLen)
			{
				/* ACK to the received byte */
				I2C_AcknowledgeConfig(bus, ENABLE);
			}
			else
			{
				/* not ACK to the last byte*/
				I2C_AcknowledgeConfig(bus, DISABLE);
			}

			/* wait receiving to finish */
			timeout = I2C_TIMEOUT_MAX;
			while(!I2C_CheckEvent(bus, I2C_EVENT_MASTER_BYTE_RECEIVED))
			{
				if(!(timeout--))
				{
					ret = 3;
					goto error;
				}
			}

			*(data.read++) = I2C_ReceiveData(bus);
		}
		break;
	case I2C_WriteOnly:
		if(data.write == NULL)
		{
			goto error;
		}
		/* step 2 -> send the slave address+w */
		timeout = I2C_TIMEOUT_MAX;
		I2C_Send7bitAddress(bus, (uint8_t)((data.slave << 1)), I2C_Direction_Transmitter);

		while(!I2C_CheckEvent(bus, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
		{
			if(!(timeout--))
			{
				ret = 4;
				goto error;
			}
		}

		/* step 3 -> send the data to the slave */
		while(data.wLen--)
		{
			timeout = I2C_TIMEOUT_MAX;
			I2C_SendData(bus, *(data.write++));
			while(!I2C_CheckEvent(bus,I2C_EVENT_MASTER_BYTE_TRANSMITTED))
			{
				if(!(timeout--))
				{
					ret = 5;
					goto error;
				}
			}
		}
		break;
	case I2C_WriteRead:
		if(data.read == NULL || data.write == NULL)
		{
			goto error;
		}
		/* step 2 -> send the slave address+w */
		timeout = I2C_TIMEOUT_MAX;
		I2C_Send7bitAddress(bus, (uint8_t)((data.slave << 1)), I2C_Direction_Transmitter);

		while(!I2C_CheckEvent(bus, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
		{
			if(!(timeout--))
			{
				ret = 6;
				goto error;
			}
		}

		/* step 3 -> send the data to the slave*/
		while(data.wLen--)
		{
			timeout = I2C_TIMEOUT_MAX;
			I2C_SendData(bus, *(data.write++));
			while(!I2C_CheckEvent(bus,I2C_EVENT_MASTER_BYTE_TRANSMITTED))
			{
				if(!(timeout--))
				{
					ret = 7;
					goto error;
				}
			}
		}

		/* step 4 -> generate a restart bit */
		bus->SR1 |= (uint16_t)0x0400; /* Clear AF flag if arised */
		I2C_GenerateSTART(bus, ENABLE);

		timeout = I2C_TIMEOUT_MAX;
		while(!I2C_CheckEvent(bus, I2C_EVENT_MASTER_MODE_SELECT))
		{
			if(!(timeout--))
			{
				ret = 8;
				goto error;
			}
		}

		/* step 5 -> send the slave address+r */
		I2C_Send7bitAddress(bus, (uint8_t)((data.slave << 1)|0x01), I2C_Direction_Receiver);

		timeout = I2C_TIMEOUT_MAX;
		while(!I2C_CheckEvent(bus, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
		{
			if(!(timeout--))
			{
				ret = 9;
				goto error;
			}
		}


		/* step 6 -> to receive the data */
		while(data.rLen--)
		{
			if(data.rLen)
			{
				/* ACK to the received byte */
				I2C_AcknowledgeConfig(bus, ENABLE);
			}
			else
			{
				/* not ACK to the last byte*/
				I2C_AcknowledgeConfig(bus, DISABLE);
			}

			timeout = I2C_TIMEOUT_MAX;
			while(!I2C_CheckEvent(bus, I2C_EVENT_MASTER_BYTE_RECEIVED))
			{
				if(!(timeout--))
				{
					ret = 10;
					goto error;
				}
			}

			*(data.read++) = I2C_ReceiveData(bus);
		}

		break;
	}
error:
	/* step n -> send the stop bit */
	I2C_GenerateSTOP(bus, ENABLE);
	return ret;
}
#endif
 uint32_t RTC3232_Get_Time(Time_Typedef *TimeVAL)
{
	I2C_TransmiData_t data;
	uint32_t ret = 0;
	uint8_t write[2];
	uint8_t read[8];
	write[0] = Address_second;

	data.dir = I2C_WriteRead;
	data.slave = DS3232_ADDRESS;
	data.write = write;
	data.read  = read;
	data.wLen  = 1;
	data.rLen  = 7;

	ret = I2C_Transmit(I2C2, data);

	TimeVAL->second = RTC_Bcd2ToByte(data.read[0] & Shield_secondBit);
	TimeVAL->minute  = RTC_Bcd2ToByte(data.read[1] & Shield_minuteBit);
	TimeVAL->hour      = RTC_Bcd2ToByte(data.read[2] & Shield_hourBit);
	TimeVAL->week     = RTC_Bcd2ToByte(data.read[3] & Shield_weekBit);	
	TimeVAL->date      = RTC_Bcd2ToByte(data.read[4] & Shield_dateBit);	
	TimeVAL->month   = RTC_Bcd2ToByte(data.read[5] & Shield_monthBit);	
	TimeVAL->year      = RTC_Bcd2ToByte(data.read[6] | DS3232_YEARDATA);
	//sprintf(RtcDebugBuffer, "DS3232 Read time: second is: %d   minute is: %d   year is :  %d\r\n",
	//		TimeVAL->second,TimeVAL->minute,TimeVAL->year);
	//dbg_printf(RtcDebugBuffer);

	return ret;
}
 uint32_t RTC_Check(void)
{
	I2C_TransmiData_t data;
	uint32_t ret = 0;
	uint8_t write[2];
	uint8_t read[2];

	write[0] = Address_control_status;

	data.dir = I2C_WriteRead;
	data.slave = DS3232_ADDRESS;
	data.write = write;
	data.read  = read;
	data.wLen  = 1;
	data.rLen  = 1;

	ret= I2C_Transmit(I2C2, data);
	if (read[0]&0x80)
	{
	ret=1;
        sprintf(RtcDebugBuffer, "DS3232 has stop************ \r\n");
	dbg_printf(RtcDebugBuffer);
	}
	
	return ret;
}

 uint32_t RTC_EOSC_Enable(void)
{
	I2C_TransmiData_t data;
	uint32_t ret = 0;
	uint8_t write[2];
	uint8_t read[2];

	write[0] = Address_control;
	write[1] = 0x00;

	data.dir = I2C_WriteOnly;
	data.slave = DS3232_ADDRESS;
	data.write = write;
	data.read  = read;
	data.wLen  = 2;
	data.rLen  = 0;

	ret= I2C_Transmit(I2C2, data);
	return ret;
}

 uint32_t RTC_TimeInit(Time_Typedef  *TimeVAL)
{
	I2C_TransmiData_t data;
	uint32_t ret = 0;
	uint8_t write[10];
	uint8_t read[2];

	write[0] = Address_second;
	write[1] = (uint8_t)RTC_ByteToBcd2(TimeVAL->second);
	write[2] = (uint8_t)RTC_ByteToBcd2(TimeVAL->minute);
	write[3] = (uint8_t)RTC_ByteToBcd2(TimeVAL->hour);
	write[4] = (uint8_t)RTC_ByteToBcd2(TimeVAL->week);
	write[5] = (uint8_t)RTC_ByteToBcd2(TimeVAL->date);
	write[6] = (uint8_t)RTC_ByteToBcd2(TimeVAL->month);
	write[7] = (uint8_t)RTC_ByteToBcd2((uint8_t)(TimeVAL->year));
	data.dir = I2C_WriteOnly;
	data.slave = DS3232_ADDRESS;
	data.write = write;
	data.read  = read;
	data.wLen  = 8;
	data.rLen  = 0;

	ret= I2C_Transmit(I2C2, data);
	return ret;
}
#if 0
void RTC3232_task(void* p)
{    
       Time_Typedef TimeVAL,temp;
	// TimeVAL.year=2017;//2017.04.24   4.27.10
	// TimeVAL.month=4;
	 //TimeVAL.date=24;
	// TimeVAL.hour=4;
	 //TimeVAL.minute=27;
	 //TimeVAL.second=10;
         //RTC_TimeInit(&TimeVAL);
	  RTC_EOSC_Enable();
       //dbg_printf("RTC3232_task is running!\r\n");
	while(1)
	{
		
		//if(!RTC_Check())
		{
               //dbg_printf("RTC3232_task is running!\r\n");
               RTC3232GetTime(&temp);
		}
		
	    vTaskDelay(1500); /* 1000ms */
	}

	vTaskDelete(NULL);
}
#endif
