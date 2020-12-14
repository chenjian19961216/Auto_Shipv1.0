#include "gps.h"

/***************************GPS功能代码*******************************/
static struct rt_semaphore gps_sem;
static rt_device_t serial_gps;
static rt_uint8_t buff_gps[80];


static rt_err_t gps_callback(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&gps_sem);
    return RT_EOK;
}


static void gps_thread_entry(void *parameter)
{
	
	rt_kprintf("GPS initialized!\r\n");
	ship_state.gps_sta=1;
	
	while(1)
	{
		rt_sem_take(&gps_sem, RT_WAITING_FOREVER);
		if(rt_device_read(serial_gps, 0, buff_gps, 80))
		{
			if(buff_gps[0]=='$')
			{
				NMEA_GNRMC_Analysis(&(ship_sensor.gps_data),(rt_uint8_t*)buff_gps);
			}
			rt_memset(buff_gps,0,80);
		}
	}
}


static int gps_init(void)
{
		 rt_err_t ret = RT_EOK;
     char uart_name[RT_NAME_MAX];
     rt_strncpy(uart_name, GPS_UART_NAME, RT_NAME_MAX);

		 struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */

     /* 查找串口设备 */
     serial_gps = rt_device_find(uart_name);
     if (!serial_gps)
     {
         rt_kprintf("find %s failed!\n", uart_name);
         return RT_ERROR;
     }
		 	/* step2：修改串口配置参数 */
			config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
			config.data_bits = DATA_BITS_8;           //数据位 8
			config.stop_bits = STOP_BITS_1;           //停止位 1
			config.bufsz     = 256;                   //修改缓冲区 buff size 为 128
			config.parity    = PARITY_NONE;           //无奇偶校验位
		 rt_device_control(serial_gps, RT_DEVICE_CTRL_CONFIG, &config);
		 
		 /* 初始化信号量 */
		 rt_sem_init(&gps_sem, "gps_sem", 0, RT_IPC_FLAG_FIFO);
     /* 以 DMA 接收及轮询发送方式打开串口设备 */
     rt_device_open(serial_gps, RT_DEVICE_FLAG_DMA_RX);
     /* 设置接收回调函数 */
     rt_device_set_rx_indicate(serial_gps, gps_callback);

		rt_thread_t thread = rt_thread_create("gps", gps_thread_entry, RT_NULL, 2048, 6, 10);
    /* 创建成功则启动线程 */
		if(thread != RT_NULL)				
			rt_thread_startup(thread);
    else						            
			ret = RT_ERROR;
    return ret;
}
INIT_APP_EXPORT(gps_init);



//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//       0XFF,代表不存在第cx个逗号							  
rt_uint8_t NMEA_Comma_Pos(rt_uint8_t *buf,rt_uint8_t cx)
{	 		    
	rt_uint8_t *p=buf;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;	 
}

//m^n函数
//返回值:m^n次方.
rt_uint32_t NMEA_Pow(rt_uint8_t m,rt_uint8_t n)
{
	rt_uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}
//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
int NMEA_Str2num(rt_uint8_t *buf,rt_uint8_t*dx)
{
	rt_uint8_t *p=buf;
	rt_uint32_t ires=0,fres=0;
	rt_uint8_t ilen=0,flen=0,i;
	rt_uint8_t mask=0;
	int res;
	while(1) //得到整数和小数的长度
	{
		if(*p=='-'){mask|=0X02;p++;}//是负数
		if(*p==','||(*p=='*'))break;//遇到结束了
		if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
		else if(*p>'9'||(*p<'0'))	//有非法字符
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//去掉负号
	for(i=0;i<ilen;i++)	//得到整数部分数据
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//最多取5位小数
	*dx=flen;	 		//小数点位数
	for(i=0;i<flen;i++)	//得到小数部分数据
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;		   
	return res;
}	
//分析GNRMC信息
//gpsx:gps信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GNRMC_Analysis(GPS_DATA* gpsx,rt_uint8_t *buf)
{
	rt_uint8_t *p1,dx;			 
	rt_uint8_t posx; 
	rt_uint32_t temp;	   
	float rs;   
	p1=(rt_uint8_t*)strstr((const char *)buf,"$GNRMC");
	posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
		gpsx->D.hour = temp/10000;
		gpsx->D.minute=(temp/100)%100;
		gpsx->D.second=temp%100;	 	 
	}	
	posx=NMEA_Comma_Pos(p1,3);								//得到纬度
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		gpsx->latitude=(gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60)/100000;//转换为° 
	}
	posx=NMEA_Comma_Pos(p1,4);								//南纬还是北纬 
	if(posx!=0XFF)gpsx->NS=*(p1+posx);
	posx=NMEA_Comma_Pos(p1,5);								//得到经度
	if(posx!=0XFF)
	{												  
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		gpsx->longitude=(gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60)/100000;//转换为°
	posx=NMEA_Comma_Pos(p1,6);								//东经还是西经
	if(posx!=0XFF)gpsx->EW=*(p1+posx);	
	posx=NMEA_Comma_Pos(p1,9);								//得到UTC日期
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//得到UTC日期
		gpsx->D.day=temp/10000;
		gpsx->D.month=(temp/100)%100;
		gpsx->D.year=2000+temp%100;	 	 
	} 		
	}
	
}











//static void UTC2BTC(date_time *GPS)
//{
////***************************************************
////如果秒号先出,再出时间数据,则将时间数据+1秒
//		GPS->second++; //加一秒
//		if(GPS->second>59){
//			GPS->second=0;
//			GPS->minute++;
//			if(GPS->minute>59){
//				GPS->minute=0;
//				GPS->hour++;
//			}
//		}	
// 
////***************************************************
//		GPS->hour+=8;		//北京时间跟UTC时间相隔8小时
//		if(GPS->hour>23)
//		{
//			GPS->hour-=24;
//			GPS->day+=1;
//			if(GPS->month==2 ||GPS->month==4 ||GPS->month==6 ||GPS->month==9 ||GPS->month==11 ){
//				if(GPS->day>30){			//上述几个月份是30天每月，2月份还不足30
//			   		GPS->day=1;
//					GPS->month++;
//				}
//			}
//			else{
//				if(GPS->day>31){			//剩下的几个月份都是31天每月
//			   		GPS->day=1;
//					GPS->month++;
//				}
//			}
//			if(GPS->year % 4 == 0 ){//
//		   		if(GPS->day > 29 && GPS->month ==2){		//闰年的二月是29天
//		   			GPS->day=1;
//					GPS->month++;
//				}
//			}
//			else{
//		   		if(GPS->day>28 &&GPS->month ==2){		//其他的二月是28天每月
//		   			GPS->day=1;
//					GPS->month++;
//				}
//			}
//			if(GPS->month>12){
//				GPS->month-=12;
//				GPS->year++;
//			}		
//		}
//}

/***************************计算两点之间距离*******************************/
double Get_distance(double lat1, double lng1, double lat2, double lng2)
{
	double radLat1 = lat1 * PI / 180.0;   //角度1? = π / 180
	double radLat2 = lat2 * PI / 180.0;   //角度1? = π / 180
	double a = radLat1 - radLat2;//纬度之差
	double b = lng1 * PI / 180.0 - lng2* PI / 180.0;  //经度之差
	double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2))));
	dst = dst * EARTH_R;
	dst = round(dst * 10000) / 10000;
	return dst;
}
 
/***************************计算两点连线与基准轴夹角*******************************/
double Get_angle(double lat1, double lng1, double lat2, double lng2)
{
	double x = lat1 - lat2;//t d
	double y = lng1 - lng2;//z y
	int angle=-1;
	if (y == 0 && x > 0) angle = 0;
	if (y == 0 && x < 0) angle = 180;
	if(x ==0 && y > 0) angle = 90;
	if(x == 0 && y < 0) angle = 270;
	if (angle == -1)
	{
		double dislat = Get_distance(lat1, lng2, lat2, lng2);
		double dislng = Get_distance(lat2, lng1, lat2, lng2);
		if (x > 0 && y > 0) angle = atan2(dislng, dislat) / PI * 180;
		if (x < 0 && y > 0) angle = atan2(dislat, dislng) / PI * 180+90;
		if (x < 0 && y < 0) angle = atan2(dislng, dislat) / PI * 180 + 180;
		if (x > 0 && y < 0) angle = atan2(dislat, dislng) / PI * 180 + 270;
	}
	return angle;
}

