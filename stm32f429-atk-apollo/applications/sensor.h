#ifndef __SENSOR_H__
#define __SENSOR_H__
#include <rtthread.h>
#include <string.h>
#include <rtdevice.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>




#define EARTH_R 6371004//地球半径（m）
#define PI 3.14159






/*时间结构体*/
typedef struct
{
	int year;  
	int month; 
	int day;
	int hour;
	int minute;
	int second;
}date_time;


/*GPS数据结构体*/
typedef struct
{
	 date_time D;
	 char status;  		
	 double	latitude;   
	 double longitude; 
	 double	latitude_wf;   
	 double longtitude_wf; 
	 char NS;           
	 char EW;           
	 double speed;      
	 double high; 
}GPS_DATA;

/*IMU数据结构体*/
typedef struct
{
	float accelx;
	float accely;
	float accelz;
	float ang_vx;
	float ang_vy;
	float ang_vz;
	float roll;
	float pitch;
	float yaw;
}IMU_DATA;

/*水质数据结构体*/
typedef struct
{
	float ph;
	float temp;
	float elec;
}WATER_DATA;


/*传感器数据结构体*/
typedef struct
{
	IMU_DATA 		imu_data;
	GPS_DATA 	 	gps_data;
	WATER_DATA	water_data;
	
}SENSOR;

/*传感器状态结构体*/
typedef struct
{
	rt_uint8_t imu_sta;
	rt_uint8_t gps_sta;
	rt_uint8_t ultra_sta;	
}STATE;


/*无人船姿态解算结构体*/
typedef struct
{
	double start_lon;
	double start_la;
	double tar_lon;
	double tar_la;
	double distance;
	double angle;
	double dev_angle_min;			//最小偏差角（0-180）
	double dev_angle_min_sign;	//最小偏差角（-180——180）
	double dev_angle;				//船的航向角与目标角度的偏差
	double dev_angle_last; 
	double dev_angle_change;
	double dev_angle_sum;
	double dev_angle_tar_imu;
	double dev_angle_tar_gps;
	double dev_angle_tar_gps_change;
	double distance_ek;
}TARGET;


extern SENSOR ship_sensor;
extern TARGET ship_target;
extern STATE ship_state;
extern double Get_distance(double lng1,double lat1,double lng2,double lat2);
extern double Get_angle(double lng1,double lat1,double lng2,double lat2);
extern void PrintFloat(float value);
extern unsigned short CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen);
extern void Info_Print(void);



#endif


