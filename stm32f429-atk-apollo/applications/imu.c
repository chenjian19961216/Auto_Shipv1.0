#include "imu.h"





/***************************IMU���ܴ���*******************************/
#define G 9.8
static struct rt_semaphore imu_sem;
static rt_device_t serial_imu;
static rt_uint8_t buff_imu[50];

static rt_err_t imu_callback(rt_device_t dev, rt_size_t size)
{
    /* ���ڽ��յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
						//rt_kprintf("%d\r\n",size);
					
						rt_sem_release(&imu_sem);

		return RT_EOK;		
				
		
    
}

static void imu_thread_entry(void *parameter)
{
		//rt_uint8_t cmd[5]={0xFF,0xAA,0x02,0x0E,0x00};
		rt_kprintf("IMU initialized!\r\n");
		ship_state.imu_sta=1;

    while (1)
    {
				//�ȴ��ź���
				rt_sem_take(&imu_sem, RT_WAITING_FOREVER);
					if(rt_device_read(serial_imu, 0, buff_imu, 10)==10) 
					{
						if(buff_imu[0]==0x55)		//����ͷ
						{
								if(buff_imu[1]==0x53)	//�Ƕ�����
								{
									ship_sensor.imu_data.yaw = (float)(short)((buff_imu[7]<<8)|buff_imu[6])/32768*180;
								}
														
							rt_memset(buff_imu,0,50);	
						}
							
//						if(CRC16_MODBUS(buff_imu,27)==((buff_imu[28]<<8)|buff_imu[27]))
//						{
//							ship_pos.imu_data.accelx=(float)((short)(buff_imu[3]<<8)|buff_imu[4])/32768*2*G;
//							ship_pos.imu_data.accely=(float)((short)(buff_imu[5]<<8)|buff_imu[6])/32768*2*G;
//							ship_pos.imu_data.accelz=(float)((short)(buff_imu[7]<<8)|buff_imu[8])/32768*2*G;
//							ship_pos.imu_data.ang_vx=(float)((short)(buff_imu[9]<<8)|buff_imu[10])/32768*250;
//							ship_pos.imu_data.ang_vy=(float)((short)(buff_imu[11]<<8)|buff_imu[12])/32768*250;
//							ship_pos.imu_data.ang_vz=(float)((short)(buff_imu[13]<<8)|buff_imu[14])/32768*250;
//							if((float)((short)(buff_imu[25]<<8)|buff_imu[26])/32768*180<=0)
//								ship_pos.imu_data.yaw=(float)(-1)*((short)(buff_imu[25]<<8)|buff_imu[26])/32768*180;
//							else
//								ship_pos.imu_data.yaw=360-(float)((short)(buff_imu[25]<<8)|buff_imu[26])/32768*180;
//						}
					}	

					//tar_pos.dev_angle_tar_imu=ship_pos.imu_data.yaw-tar_pos.dev_angle_tar_gps;
				}
			

}

static int imu_init(void)
{
		 rt_err_t ret = RT_EOK;
     char uart_name[RT_NAME_MAX];
     rt_strncpy(uart_name, IMU_UART_NAME, RT_NAME_MAX);

		 struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* ��ʼ�����ò��� */

     /* ���Ҵ����豸 */
     serial_imu = rt_device_find(uart_name);
     if (!serial_imu)
     {
         rt_kprintf("find %s failed!\n", uart_name);
         return RT_ERROR;
     }
		 	/* step2���޸Ĵ������ò��� */
			config.baud_rate = BAUD_RATE_9600;        //�޸Ĳ�����Ϊ 9600
			config.data_bits = DATA_BITS_8;           //����λ 8
			config.stop_bits = STOP_BITS_1;           //ֹͣλ 1
			config.bufsz     = 256;                   //�޸Ļ����� buff size Ϊ 256
			config.parity    = PARITY_NONE;           //����żУ��λ
		 rt_device_control(serial_imu, RT_DEVICE_CTRL_CONFIG, &config);
		 
		 /* ��ʼ���ź��� */
		 rt_sem_init(&imu_sem, "imu_sem", 0, RT_IPC_FLAG_FIFO);
     /* ��DMA���ռ���ѯ���ͷ�ʽ�򿪴����豸 */
     rt_device_open(serial_imu, RT_DEVICE_FLAG_DMA_RX);
     /* ���ý��ջص����� */
     rt_device_set_rx_indicate(serial_imu, imu_callback);

		rt_thread_t thread = rt_thread_create("imu", imu_thread_entry, RT_NULL, 2048, 5, 10);
    /* �����ɹ��������߳� */
		if(thread != RT_NULL)				
			rt_thread_startup(thread);
    else						            
			ret = RT_ERROR;
    return ret;
}
INIT_APP_EXPORT(imu_init);

///***************************����IMUУ׼����*******************************/
//static int Correct_func(int argc, char *argv[])
//{
//    rt_err_t ret = RT_EOK;
//		char func[RT_NAME_MAX];

//		if (argc < 2)
//    {
//			rt_kprintf("Please input:Correct_func <mag or accel>\r\n");
//      return RT_ERROR;
//    }		 
//		rt_strncpy(func, argv[1], RT_NAME_MAX);
//		if(rt_strcmp(func,"mag")==0)
//		{
//			cor_obj=1;
//		}
//		else if(rt_strcmp(func,"accel")==0)
//		{
//			cor_obj=2;
//		}
//		return ret;
//}
///* ������ msh �����б��� */
//MSH_CMD_EXPORT(Correct_func, Correct function);
