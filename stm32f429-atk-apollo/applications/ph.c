#include "ph.h"

/***************************PH�ƹ��ܴ���*******************************/
#define I2C_BUS    "i2c1"	
static struct rt_semaphore ph_sem;
static rt_device_t serial_ph;
static rt_uint8_t buff_ph[50];




float data2hex(rt_uint32_t data)  
{

	float b=*(float*)&data;
	return b;
}




static rt_err_t ph_callback(rt_device_t dev, rt_size_t size)
{
    /* ���ڽ��յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
    rt_sem_release(&ph_sem);
    return RT_EOK;
}

static void ph_thread_entry(void *parameter)
{
	rt_uint32_t recv = 0;
	rt_uint8_t cmd[] = {0x01,0x03,0x00,0x01,0x00,0x02,0x95,0xCB};
	pcf8574_device_t dev = RT_NULL; 
  dev = pcf8574_init(I2C_BUS, RT_NULL);
	
	rt_kprintf("PH initialized!\r\n");
	
	while(1)
	{
		
		rt_device_write(serial_ph,0,cmd,8);
		pcf8574_pin_write(dev,6,0);		//485��Ϊ����ģʽ
		rt_sem_take(&ph_sem, RT_WAITING_FOREVER);
		if(rt_device_read(serial_ph, 0, buff_ph, 9)==9)
		{
			if(buff_ph[0]==0x01&&buff_ph[1]==0x03&&buff_ph[2]==0x04)
			{
			recv = (buff_ph[5]<<24)|(buff_ph[6]<<16)|(buff_ph[3]<<8)|buff_ph[4];
			//rt_kprintf("%x,%x,%x,%x",buff_ph[5],buff_ph[6],buff_ph[3],buff_ph[4]);
			ship_sensor.water_data.ph=data2hex(recv);//ʮ������ת��Ϊ������
			}

			rt_memset(buff_ph,0,50);
		}
		pcf8574_pin_write(dev,6,1);		//485��Ϊ����ģʽ
		rt_thread_mdelay(100);
	}
}

static int ph_init(void)
{
		 rt_err_t ret = RT_EOK;
     char uart_name[RT_NAME_MAX];
     rt_strncpy(uart_name, PH_UART_NAME, RT_NAME_MAX);

		 struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* ��ʼ�����ò��� */

     /* ���Ҵ����豸 */
     serial_ph = rt_device_find(uart_name);
     if (!serial_ph)
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
		 rt_device_control(serial_ph, RT_DEVICE_CTRL_CONFIG, &config);
		 
		 /* ��ʼ���ź��� */
		 rt_sem_init(&ph_sem, "ph_sem", 0, RT_IPC_FLAG_FIFO);
     /* �� DMA ���ռ���ѯ���ͷ�ʽ�򿪴����豸 */
     rt_device_open(serial_ph, RT_DEVICE_FLAG_DMA_RX);
     /* ���ý��ջص����� */
     rt_device_set_rx_indicate(serial_ph, ph_callback);

		rt_thread_t thread = rt_thread_create("ph", ph_thread_entry, RT_NULL, 2048, 6, 10);
    /* �����ɹ��������߳� */
		if(thread != RT_NULL)				
			rt_thread_startup(thread);
    else						            
			ret = RT_ERROR;
    return ret;
}
INIT_APP_EXPORT(ph_init);