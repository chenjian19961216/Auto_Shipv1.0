#include "sensor.h"


SENSOR ship_sensor;	//定义船体传感器数据结构体指针
TARGET ship_target;	//定义船体传感器解算数据结构体指针
STATE ship_state;	  //定义船体传感器状态结构体指针




/***************************打印浮点数*******************************/
void PrintFloat(float value)
{
    int tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6;
    tmp = (int)value;
		if(value-tmp<0)
		{	
			tmp1=(unsigned int)((tmp-value)*10)%10;
			tmp2=(unsigned int)((tmp-value)*100)%10;
			tmp3=(unsigned int)((tmp-value)*1000)%10;
			tmp4=(unsigned int)((tmp-value)*10000)%10;
			tmp5=(unsigned int)((tmp-value)*100000)%10;
			tmp6=(unsigned int)((tmp-value)*1000000)%10;
			rt_kprintf("%d.%d%d%d%d%d%d\r\n",tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);
		}
		else
		{
			tmp1=(unsigned int)((value-tmp)*10)%10;
			tmp2=(unsigned int)((value-tmp)*100)%10;
			tmp3=(unsigned int)((value-tmp)*1000)%10;
			tmp4=(unsigned int)((value-tmp)*10000)%10;
			tmp5=(unsigned int)((value-tmp)*100000)%10;
			tmp6=(unsigned int)((value-tmp)*1000000)%10;
			rt_kprintf("%d.%d%d%d%d%d%d\r\n",tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);
		}
}


/***************************CRC-modbus校验函数*******************************/
#define CRC_POLY 0X8005
void InvertUint8(unsigned char *DesBuf, unsigned char *SrcBuf)
{
		int i;
		unsigned char temp = 0;
		
		for(i = 0; i < 8; i++)
		{
				if(SrcBuf[0] & (1 << i))
				{
						temp |= 1<<(7-i);
				}
		}
		DesBuf[0] = temp;
}
 
void InvertUint16(unsigned short *DesBuf, unsigned short *SrcBuf)  
{  
		int i;  
		unsigned short temp = 0;    
		
		for(i = 0; i < 16; i++)  
		{  
				if(SrcBuf[0] & (1 << i))
				{          
						temp |= 1<<(15 - i);  
				}
		}  
		DesBuf[0] = temp;  
}

unsigned short CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen)  
{  
	 unsigned short wCRCin = 0xFFFF;  
	 unsigned short wCPoly = 0x8005;  
	 unsigned char wChar = 0;  
	 
	 while (usDataLen--)     
	 {  
			 wChar = *(puchMsg++);  
			 InvertUint8(&wChar, &wChar);  
			 wCRCin ^= (wChar << 8); 
			 
			 for(int i = 0; i < 8; i++)  
			 {  
					 if(wCRCin & 0x8000) 
					 {
							 wCRCin = (wCRCin << 1) ^ wCPoly;  
					 }
					 else  
					 {
							 wCRCin = wCRCin << 1; 
					 }            
			 }  
	 }  
	 InvertUint16(&wCRCin, &wCRCin);  
	 return (wCRCin) ;  
	 
} 

void Info_Print(void)
{
	//gps
	rt_kprintf("GPS INFO:");
	rt_kprintf("%d-%d-%d %d:%d:%d\r\n",ship_sensor.gps_data.D.year,ship_sensor.gps_data.D.month,ship_sensor.gps_data.D.day,ship_sensor.gps_data.D.hour,ship_sensor.gps_data.D.minute,ship_sensor.gps_data.D.second);
	rt_kprintf("%c ",ship_sensor.gps_data.NS);
	PrintFloat(ship_sensor.gps_data.latitude);
	rt_kprintf("%c ",ship_sensor.gps_data.EW);
	PrintFloat(ship_sensor.gps_data.longitude);

	//imu
//	rt_kprintf("\r\nIMU INFO:\r\naccelx:");
//	PrintFloat(ship_sensor.imu_data.accelx);
//	rt_kprintf("accely:");
//	PrintFloat(ship_pos.imu_data.accely);
//	rt_kprintf("accelz:");
//	PrintFloat(ship_pos.imu_data.accelz);
//	rt_kprintf("ang_vx:");
//	PrintFloat(ship_pos.imu_data.ang_vx);
//	rt_kprintf("ang_vy:");
//	PrintFloat(ship_pos.imu_data.ang_vy); 
//	rt_kprintf("ang_vz:");
//	PrintFloat(ship_pos.imu_data.ang_vz);
	rt_kprintf("yaw:");
	PrintFloat(ship_sensor.imu_data.yaw);
	
	rt_kprintf("PH:");
	PrintFloat(ship_sensor.water_data.ph);
////	//ultrasound
////	rt_kprintf("\r\nULT INFO:\r\n");
////	rt_kprintf("ult1:%d\r\n",ship_pos.ultra_data.ultra1);
////	rt_kprintf("ult2:%d\r\n",ship_pos.ultra_data.ultra2);
////  rt_kprintf("ult3:%d\r\n",ship_pos.ultra_data.ultra3);
////	rt_kprintf("ult4:%d\r\n",ship_pos.ultra_data.ultra4);
////	//target
//	rt_kprintf("target distance:");
//	PrintFloat(tar_pos.distance);
//	rt_kprintf("target angle:");
//	PrintFloat(tar_pos.angle);
//	rt_kprintf("deviation angle:");
//	PrintFloat(tar_pos.dev_angle);
////	rt_kprintf("deviation angle(imu-tar):");
////	PrintFloat(tar_pos.dev_angle_tar_imu);
//	//rt_kprintf("deviation angle(gps-tar):");
//	//PrintFloat(tar_pos.dev_angle_tar_gps_change);
//	//rt_kprintf("distance_ek:");
//	//PrintFloat(tar_pos.distance_ek);
//	//PrintFloat(tar_pos.dev_angle_tar_gps);
//	//rt_kprintf("last deviation angle:");
//	//PrintFloat(tar_pos.dev_angle_last);
////	rt_kprintf("deviation angle_min_sign_last:");
////	PrintFloat(tar_pos.dev_angle_last);
////	rt_kprintf("deviation angle_min_sign:");
////	PrintFloat(tar_pos.dev_angle_min_sign);
////	rt_kprintf("deviation angle change:");
////	PrintFloat(tar_pos.dev_angle_change);
////	rt_kprintf("deviation angle sum:");
////	PrintFloat(tar_pos.dev_angle_sum);
}

