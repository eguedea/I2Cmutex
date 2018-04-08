/*
 * I2C.c
 *
 *  Created on: Mar 26, 2018
 *      Author: eric_
 */
#include "I2C.h"
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "fsl_i2c.h"
#include "FREErtos.h"
#include "event_groups.h"
#include "semphr.h"

#define EEPROMADDRESS (0x57)
#define SECONDSADDRESS (0x02)
#define MINUTESADDRESS (0x03)
#define HOURSADDRESS (0x04)
#define DAYADDRESS (0x05)
#define MONTHADDRESS (0x06)
#define YEARADDRESS (0x05)
#define RTCADDRESS (0x50)
#define ADDRESS_SIZE (1)
#define MAXSECONDS (60)
#define MAXMINUTES (60)
#define MAXHOURS     (23)
#define MAXDAYS     (31)
#define MAXMONTHS     (12)

volatile bool g_MasterCompletionFlag = false;
i2c_master_handle_t g_m_handle;
SemaphoreHandle_t i2cSemaphore;
SemaphoreHandle_t i2cMutex;

static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle,
        status_t status, void * userData)
{

	if (status == kStatus_Success)
	{
		g_MasterCompletionFlag = true;
		xSemaphoreGive(i2cSemaphore);

	}

}
void I2Cinit()
{
	CLOCK_EnableClock(kCLOCK_PortB);
	CLOCK_EnableClock(kCLOCK_I2c0);

	port_pin_config_t config_i2c =
		{ kPORT_PullUp, kPORT_SlowSlewRate, kPORT_PassiveFilterDisable,
		        kPORT_OpenDrainEnable, kPORT_LowDriveStrength, kPORT_MuxAlt2,
		        kPORT_UnlockRegister, };

		PORT_SetPinConfig(PORTB, 2, &config_i2c);
		PORT_SetPinConfig(PORTB, 3, &config_i2c);
	i2c_master_config_t masterConfig;
	I2C_MasterGetDefaultConfig(&masterConfig);
	masterConfig.baudRate_Bps = 100000;
	I2C_MasterInit(I2C0, &masterConfig, CLOCK_GetFreq(kCLOCK_BusClk));
	i2cSemaphore = xSemaphoreCreateBinary();
	i2cMutex = xSemaphoreCreateMutex();
}
/*
 * EEPROM 24LC256 FUNCTIONS
 */
void writeMemory(uint16_t add,uint8_t data)
{

	i2c_master_config_t masterConfig;
	i2c_master_transfer_t masterXfer;

	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

	uint8_t datah = data;
	xSemaphoreTake(i2cMutex, portMAX_DELAY);

	masterXfer.slaveAddress = EEPROMADDRESS;
	masterXfer.direction = kI2C_Write;
	masterXfer.subaddress = add;
	masterXfer.subaddressSize = 2;
	masterXfer.data = &datah;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;


	I2C_MasterTransferNonBlocking(I2C0, &g_m_handle,&masterXfer);
	I2Cwritedelay();

	while (!g_MasterCompletionFlag)
	{
	}
	g_MasterCompletionFlag = false;
	I2Cwritedelay();

	xSemaphoreGive(i2cMutex);

}



uint8_t readMemory(uint16_t add)
{
	i2c_master_transfer_t masterXfer;

	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

	uint8_t data_buffer;
	xSemaphoreTake(i2cMutex, portMAX_DELAY);

	masterXfer.slaveAddress = EEPROMADDRESS;
	masterXfer.direction = kI2C_Read;
	masterXfer.subaddress = add;
	masterXfer.subaddressSize = 2;
	masterXfer.data = &data_buffer;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferNonBlocking(I2C0,  &g_m_handle,&masterXfer);
	I2Cwritedelay();

	while (!g_MasterCompletionFlag){}
	g_MasterCompletionFlag = false;
	xSemaphoreGive(i2cMutex);

	return data_buffer;

}
/*
 *
 * RTC PCF8583 FUNCTIONS
 *
 */

uint8_t readRTC_sec()
{
	i2c_master_transfer_t masterXfer;

	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

	uint8_t seconds;
	uint8_t secondsadd = SECONDSADDRESS;
	xSemaphoreTake(i2cMutex, portMAX_DELAY);

	masterXfer.slaveAddress = RTCADDRESS;
	masterXfer.direction = kI2C_Read;
	masterXfer.subaddress = secondsadd;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &seconds;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferNonBlocking(I2C0,  &g_m_handle,&masterXfer);
	while (!g_MasterCompletionFlag){}
	g_MasterCompletionFlag = false;

	xSemaphoreGive(i2cMutex);

	return seconds;
}


uint8_t readRTC_min()
{
	i2c_master_transfer_t masterXfer;

	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

	uint8_t minutes;
	uint8_t minutesadd = MINUTESADDRESS;

	xSemaphoreTake(i2cMutex, portMAX_DELAY);

	masterXfer.slaveAddress = RTCADDRESS;
	masterXfer.direction = kI2C_Read;
	masterXfer.subaddress = minutesadd;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &minutes;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferNonBlocking(I2C0,  &g_m_handle,&masterXfer);
	while (!g_MasterCompletionFlag){}
	g_MasterCompletionFlag = false;

	xSemaphoreGive(i2cMutex);
	return minutes;

}

uint8_t readRTC_hour()
{
	i2c_master_transfer_t masterXfer;

	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

	uint8_t hours;
	uint8_t hoursadd = HOURSADDRESS;

	xSemaphoreTake(i2cMutex, portMAX_DELAY);

	masterXfer.slaveAddress = RTCADDRESS;
	masterXfer.direction = kI2C_Read;
	masterXfer.subaddress = hoursadd;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &hours;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferNonBlocking(I2C0,  &g_m_handle,&masterXfer);

	while (!g_MasterCompletionFlag){}
	g_MasterCompletionFlag = false;

	xSemaphoreGive(i2cMutex);

	return hours;

}

uint8_t readRTC_day()
{
	i2c_master_transfer_t masterXfer;

	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

	uint8_t days;
	uint8_t daysadd = DAYADDRESS;

	xSemaphoreTake(i2cMutex, portMAX_DELAY);


	masterXfer.slaveAddress = RTCADDRESS;
	masterXfer.direction = kI2C_Read;
	masterXfer.subaddress = daysadd;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &days;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferNonBlocking(I2C0,  &g_m_handle,&masterXfer);

	while (!g_MasterCompletionFlag){}
	g_MasterCompletionFlag = false;
	xSemaphoreGive(i2cMutex);

	return days;

}

uint8_t readRTC_month()
{
	i2c_master_transfer_t masterXfer;

	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

	uint8_t months;
	uint8_t monthsadd = MONTHADDRESS;

	xSemaphoreTake(i2cMutex, portMAX_DELAY);


	masterXfer.slaveAddress = RTCADDRESS;
	masterXfer.direction = kI2C_Read;
	masterXfer.subaddress = monthsadd;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &months;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferNonBlocking(I2C0,  &g_m_handle,&masterXfer);

	while (!g_MasterCompletionFlag){}
	g_MasterCompletionFlag = false;
	xSemaphoreGive(i2cMutex);

	return months;
}





void setRTC_sec(uint8_t sec)
{
		if(sec > MAXSECONDS){
			sec = 0;
		}
		uint8_t BCDsec = ((sec/10)<<4) | (sec%10);


		i2c_master_transfer_t masterXfer;

		I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

		uint8_t seconds = BCDsec;
		xSemaphoreTake(i2cMutex, portMAX_DELAY);

		masterXfer.slaveAddress = RTCADDRESS;
		masterXfer.direction = kI2C_Write;
		masterXfer.subaddress = SECONDSADDRESS;
		masterXfer.subaddressSize = 1;
		masterXfer.data = &seconds;
		masterXfer.dataSize = 1;
		masterXfer.flags = kI2C_TransferDefaultFlag;


		I2C_MasterTransferNonBlocking(I2C0, &g_m_handle,&masterXfer);
		while (!g_MasterCompletionFlag){}
		g_MasterCompletionFlag = false;
		I2Cwritedelay();

		xSemaphoreGive(i2cMutex);

}



void setRTC_min(uint8_t min)
{
	if(min > MAXMINUTES)
	{
			min = 0;
	}
	uint8_t BCDmin = ((min/10)<<4) | (min%10);


	i2c_master_transfer_t masterXfer;

	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

	uint8_t minutes;
	minutes = BCDmin;
	xSemaphoreTake(i2cMutex, portMAX_DELAY);


	masterXfer.slaveAddress = RTCADDRESS;
	masterXfer.direction = kI2C_Write;
	masterXfer.subaddress = MINUTESADDRESS;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &BCDmin;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferNonBlocking(I2C0,  &g_m_handle,&masterXfer);

	while (!g_MasterCompletionFlag){}
	g_MasterCompletionFlag = false;
	xSemaphoreGive(i2cMutex);


}

void setRTC_hour(uint8_t hour)
{
	if(hour > MAXHOURS)
	{
		hour = 0;
	}
	uint8_t BCDhours = ((hour/10)<<4) | (hour%10);

	i2c_master_transfer_t masterXfer;

	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

	uint8_t hours = BCDhours;

	xSemaphoreTake(i2cMutex, portMAX_DELAY);

	masterXfer.slaveAddress = RTCADDRESS;
	masterXfer.direction = kI2C_Write;
	masterXfer.subaddress = HOURSADDRESS;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &hours;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferNonBlocking(I2C0,  &g_m_handle,&masterXfer);

	while (!g_MasterCompletionFlag){}
	g_MasterCompletionFlag = false;
	xSemaphoreGive(i2cMutex);


}

void setRTC_day(uint8_t day)
{
	if(day > MAXDAYS)
	{
		day = 1;
	}
	uint8_t BCDdays = ((day/10)<<4) | (day%10);


	i2c_master_transfer_t masterXfer;

	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

	uint8_t days = BCDdays;

	xSemaphoreTake(i2cMutex, portMAX_DELAY);

	masterXfer.slaveAddress = RTCADDRESS;
	masterXfer.direction = kI2C_Write;
	masterXfer.subaddress = DAYADDRESS;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &days;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferNonBlocking(I2C0,  &g_m_handle,&masterXfer);

	while (!g_MasterCompletionFlag){}
	g_MasterCompletionFlag = false;
	xSemaphoreGive(i2cMutex);

	return days;
}

void setRTC_month(uint8_t month)
{
	if(month > MAXMONTHS)
	{
		month = 1;
	}
	uint8_t BCDmonths = ((month/10)<<4) | (month%10);

	i2c_master_transfer_t masterXfer;

	I2C_MasterTransferCreateHandle(I2C0, &g_m_handle,i2c_master_callback, NULL);

	uint8_t months = BCDmonths;
	xSemaphoreTake(i2cMutex, portMAX_DELAY);

	masterXfer.slaveAddress = RTCADDRESS;
	masterXfer.direction = kI2C_Write;
	masterXfer.subaddress = MONTHADDRESS;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &months;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferNonBlocking(I2C0,  &g_m_handle,&masterXfer);

	while (!g_MasterCompletionFlag){}
	g_MasterCompletionFlag = false;
	xSemaphoreGive(i2cMutex);


}



void I2Cwritedelay()
{
	for(uint32_t i = 12000000;i==0;i--)
	{
	}

}



