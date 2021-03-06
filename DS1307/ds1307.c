/*! \file ds1307.c \brief DS1307, support for real-time clock for AVR */
//*****************************************************************************
//
//  File Name       : 'ds1307.c'
//  Title           : DS1307 real-time clock support for AVR        
//  Author          : Alan K. Duncan - Copyright (c) 2012
//  Created         : 2012-03-30
//  Modified        : 2012-04-03
//  Version         : 1.0
//  Target MCU      : Atmel AVR series
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************
#include "ds1307.h"
#include "i2c.h"

/*  base hardware address of the device */
#define DS1307_BASE_ADDRESS 0xD0

/*  register addresses  */
#define DS1307_SECONDS_ADDR		0x00
#define DS1307_MINUTES_ADDR		0x01
#define DS1307_HOURS_ADDR		0x02
#define DS1307_DAY_ADDR			0x03
#define DS1307_DATE_ADDR		0x04
#define DS1307_MONTH_ADDR		0x05
#define DS1307_YEAR_ADDR		0x06
#define DS1307_CONTROL_ADDR		0x07

/*  control bits    */
#define CH (1<<7)
#define HR (1<<6)

u08 device_data[2];

/*  private function prototypes     */
u08 ds1307_read_register(u08 reg);
void  ds1307_write_register(u08 reg,u08 data);

static unsigned int uint2bcd(unsigned int ival)
{
	return ((ival / 10) << 4) | (ival % 10);
}

void ds1307_init(DS1307HourMode mode)
{
	/*	To start the oscillator, we need to write CH = 0 (bit 7/reg 0) */
	u08 seconds = ds1307_read_register(DS1307_SECONDS_ADDR);
	seconds &= ~CH;
	ds1307_write_register(DS1307_SECONDS_ADDR,seconds);
	
	/*	set the mode */
	u08 hour = ds1307_read_register(DS1307_HOURS_ADDR);
	if( mode == kDS1307Mode12HR )
		hour &= ~HR;
	else
		hour |= HR;
	ds1307_write_register(DS1307_HOURS_ADDR, hour);
}

u08 ds1307_seconds(void)
{
	u08 seconds_h,seconds_l;
	u08 seconds = ds1307_read_register(DS1307_SECONDS_ADDR);
	/*	mask the CH bit */
	seconds &= ~CH;
	/*	get the rest of the high nibble */
	seconds_h = seconds >> 4;
	seconds_l = seconds & 0b00001111;
	return seconds_h * 10 + seconds_l;
}

u08 ds1307_minutes(void)
{
	u08 minutes_h,minutes_l;
	u08 minutes = ds1307_read_register(DS1307_MINUTES_ADDR);
	minutes_h = minutes >> 4;
	minutes_l = minutes & 0b00001111;
	return minutes_h * 10 + minutes_l;
}

u08 ds1307_hours(void)
{
	u08 hours_h, hours_l;
	u08 hours = ds1307_read_register(DS1307_HOURS_ADDR);
	if( hours & HR )
	{
		/*	24 hour mode, so mask the two upper bits */
		hours &= ~(0b11000000);	
	}
	else
	{
		/* 12 hour mode so mask the upper three bits */
		hours &= ~(0b11100000);
	}
	hours_h = hours >> 4;
	hours_l = hours & 0b00001111;
	return hours_h * 10 + hours_l;
}

u08 ds1307_date(void)
{
	u08 date_h,date_l;
	u08 date = ds1307_read_register(DS1307_DATE_ADDR);
	/*	mask the uppermost two bits */
	date &= ~(0b11000000);
	date_h = date >> 4;
	date_l = date & 0b00001111;
	return date_h * 10 + date_l;
}

u08 ds1037_month(void)
{
	u08 month_h,month_l;
	u08 month = ds1307_read_register(DS1307_MONTH_ADDR);
	month_h = month >> 4;
	month_l = month & 0b00001111;
	return month_h * 10 + month_l;
}

void ds1307_set_seconds(u08 seconds)
{
	u08 bcd_seconds = uint2bcd(seconds);
	/* make sure CH bit is clear */
	bcd_seconds &= ~CH;
	ds1307_write_register(DS1307_SECONDS_ADDR,bcd_seconds);
}

void ds1307_set_minutes(u08 minutes)
{
	u08 bcd_minutes = uint2bcd(minutes);
	/*	make sure upper bit is clear */
	bcd_minutes &= ~(1<<7);
	ds1307_write_register(DS1307_MINUTES_ADDR,bcd_minutes);
}

void ds1307_set_hours(u08 hours)
{
	u08 bcd_hours = uint2bcd(hours);
	/*	make sure upper bit is clear */
	bcd_hours &= ~(1<<7);
	u08 current_hours = ds1307_hours();
	if( hours & HR )
	{
		/*	24 hour mode so set the HR bit in bcd_hours */
		bcd_hours |= HR;
	}
	else
	{
		/* 12 hour mode so clear the HR bit in bcd_hours */
		bcd_hours &= ~HR;
	}
	ds1307_write_register(DS1307_HOURS_ADDR,bcd_hours);
}

void ds1307_set_year(u08 year)
{
	 u08 bcd_year = uint2bcd(year);
	 ds1307_write_register(DS1307_YEAR_ADDR,bcd_year);
}

void  ds1307_write_register(u08 reg,u08 data)
{
	device_data[0] = reg;
	device_data[1] = data;
	i2cMasterSendNI(DS1307_BASE_ADDRESS,2,&device_data);
}

u08 ds1307_read_register(u08 reg)
{
	device_data[0] = reg;
	i2cMasterSendNI(DS1307_BASE_ADDRESS,1,&device_data);
	i2cMasterReceiveNI(DS1307_BASE_ADDRESS,1,&device_data);
	return device_data[0];
}