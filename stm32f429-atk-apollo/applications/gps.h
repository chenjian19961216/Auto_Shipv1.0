#ifndef __GPS_H__
#define __GPS_H__
#include "sensor.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <rtthread.h>
#include <board.h>
#include <stdio.h>
#include <stdlib.h>


#define GPS_UART_NAME       "uart3"  
rt_uint8_t NMEA_Comma_Pos(rt_uint8_t *buf,rt_uint8_t cx);
rt_uint32_t NMEA_Pow(rt_uint8_t m,rt_uint8_t n);
int NMEA_Str2num(rt_uint8_t *buf,rt_uint8_t*dx);
void NMEA_GNRMC_Analysis(GPS_DATA* gpsx,rt_uint8_t *buf);
#endif

