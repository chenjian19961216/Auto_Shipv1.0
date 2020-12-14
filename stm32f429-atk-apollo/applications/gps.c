#include "gps.h"

/***************************GPS���ܴ���*******************************/
static struct rt_semaphore gps_sem;
static rt_device_t serial_gps;
static rt_uint8_t buff_gps[80];


static rt_err_t gps_callback(rt_device_t dev, rt_size_t size)
{
    /* ���ڽ��յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
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

		 struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* ��ʼ�����ò��� */

     /* ���Ҵ����豸 */
     serial_gps = rt_device_find(uart_name);
     if (!serial_gps)
     {
         rt_kprintf("find %s failed!\n", uart_name);
         return RT_ERROR;
     }
		 	/* step2���޸Ĵ������ò��� */
			config.baud_rate = BAUD_RATE_9600;        //�޸Ĳ�����Ϊ 9600
			config.data_bits = DATA_BITS_8;           //����λ 8
			config.stop_bits = STOP_BITS_1;           //ֹͣλ 1
			config.bufsz     = 256;                   //�޸Ļ����� buff size Ϊ 128
			config.parity    = PARITY_NONE;           //����żУ��λ
		 rt_device_control(serial_gps, RT_DEVICE_CTRL_CONFIG, &config);
		 
		 /* ��ʼ���ź��� */
		 rt_sem_init(&gps_sem, "gps_sem", 0, RT_IPC_FLAG_FIFO);
     /* �� DMA ���ռ���ѯ���ͷ�ʽ�򿪴����豸 */
     rt_device_open(serial_gps, RT_DEVICE_FLAG_DMA_RX);
     /* ���ý��ջص����� */
     rt_device_set_rx_indicate(serial_gps, gps_callback);

		rt_thread_t thread = rt_thread_create("gps", gps_thread_entry, RT_NULL, 2048, 6, 10);
    /* �����ɹ��������߳� */
		if(thread != RT_NULL)				
			rt_thread_startup(thread);
    else						            
			ret = RT_ERROR;
    return ret;
}
INIT_APP_EXPORT(gps_init);



//��buf����õ���cx���������ڵ�λ��
//����ֵ:0~0XFE,����������λ�õ�ƫ��.
//       0XFF,�������ڵ�cx������							  
rt_uint8_t NMEA_Comma_Pos(rt_uint8_t *buf,rt_uint8_t cx)
{	 		    
	rt_uint8_t *p=buf;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//����'*'���߷Ƿ��ַ�,�򲻴��ڵ�cx������
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;	 
}

//m^n����
//����ֵ:m^n�η�.
rt_uint32_t NMEA_Pow(rt_uint8_t m,rt_uint8_t n)
{
	rt_uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}
//strת��Ϊ����,��','����'*'����
//buf:���ִ洢��
//dx:С����λ��,���ظ����ú���
//����ֵ:ת�������ֵ
int NMEA_Str2num(rt_uint8_t *buf,rt_uint8_t*dx)
{
	rt_uint8_t *p=buf;
	rt_uint32_t ires=0,fres=0;
	rt_uint8_t ilen=0,flen=0,i;
	rt_uint8_t mask=0;
	int res;
	while(1) //�õ�������С���ĳ���
	{
		if(*p=='-'){mask|=0X02;p++;}//�Ǹ���
		if(*p==','||(*p=='*'))break;//����������
		if(*p=='.'){mask|=0X01;p++;}//����С������
		else if(*p>'9'||(*p<'0'))	//�зǷ��ַ�
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//ȥ������
	for(i=0;i<ilen;i++)	//�õ�������������
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//���ȡ5λС��
	*dx=flen;	 		//С����λ��
	for(i=0;i<flen;i++)	//�õ�С����������
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;		   
	return res;
}	
//����GNRMC��Ϣ
//gpsx:gps��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNRMC_Analysis(GPS_DATA* gpsx,rt_uint8_t *buf)
{
	rt_uint8_t *p1,dx;			 
	rt_uint8_t posx; 
	rt_uint32_t temp;	   
	float rs;   
	p1=(rt_uint8_t*)strstr((const char *)buf,"$GNRMC");
	posx=NMEA_Comma_Pos(p1,1);								//�õ�UTCʱ��
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//�õ�UTCʱ��,ȥ��ms
		gpsx->D.hour = temp/10000;
		gpsx->D.minute=(temp/100)%100;
		gpsx->D.second=temp%100;	 	 
	}	
	posx=NMEA_Comma_Pos(p1,3);								//�õ�γ��
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//�õ���
		rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
		gpsx->latitude=(gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60)/100000;//ת��Ϊ�� 
	}
	posx=NMEA_Comma_Pos(p1,4);								//��γ���Ǳ�γ 
	if(posx!=0XFF)gpsx->NS=*(p1+posx);
	posx=NMEA_Comma_Pos(p1,5);								//�õ�����
	if(posx!=0XFF)
	{												  
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//�õ���
		rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
		gpsx->longitude=(gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60)/100000;//ת��Ϊ��
	posx=NMEA_Comma_Pos(p1,6);								//������������
	if(posx!=0XFF)gpsx->EW=*(p1+posx);	
	posx=NMEA_Comma_Pos(p1,9);								//�õ�UTC����
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//�õ�UTC����
		gpsx->D.day=temp/10000;
		gpsx->D.month=(temp/100)%100;
		gpsx->D.year=2000+temp%100;	 	 
	} 		
	}
	
}











//static void UTC2BTC(date_time *GPS)
//{
////***************************************************
////�������ȳ�,�ٳ�ʱ������,��ʱ������+1��
//		GPS->second++; //��һ��
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
//		GPS->hour+=8;		//����ʱ���UTCʱ�����8Сʱ
//		if(GPS->hour>23)
//		{
//			GPS->hour-=24;
//			GPS->day+=1;
//			if(GPS->month==2 ||GPS->month==4 ||GPS->month==6 ||GPS->month==9 ||GPS->month==11 ){
//				if(GPS->day>30){			//���������·���30��ÿ�£�2�·ݻ�����30
//			   		GPS->day=1;
//					GPS->month++;
//				}
//			}
//			else{
//				if(GPS->day>31){			//ʣ�µļ����·ݶ���31��ÿ��
//			   		GPS->day=1;
//					GPS->month++;
//				}
//			}
//			if(GPS->year % 4 == 0 ){//
//		   		if(GPS->day > 29 && GPS->month ==2){		//����Ķ�����29��
//		   			GPS->day=1;
//					GPS->month++;
//				}
//			}
//			else{
//		   		if(GPS->day>28 &&GPS->month ==2){		//�����Ķ�����28��ÿ��
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

/***************************��������֮�����*******************************/
double Get_distance(double lat1, double lng1, double lat2, double lng2)
{
	double radLat1 = lat1 * PI / 180.0;   //�Ƕ�1? = �� / 180
	double radLat2 = lat2 * PI / 180.0;   //�Ƕ�1? = �� / 180
	double a = radLat1 - radLat2;//γ��֮��
	double b = lng1 * PI / 180.0 - lng2* PI / 180.0;  //����֮��
	double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2))));
	dst = dst * EARTH_R;
	dst = round(dst * 10000) / 10000;
	return dst;
}
 
/***************************���������������׼��н�*******************************/
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

