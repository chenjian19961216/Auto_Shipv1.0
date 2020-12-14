#include "engine.h"


struct rt_device_pwm *pwm_dev;      /* PWM设备句柄 */
static rt_uint32_t period = 20000000;     /* 周期为20ms，单位为纳秒ns ,频率50hz*/
static rt_uint32_t pulse_engl = 1000000;           /* 初始占空比5% 即停转*/
static rt_uint32_t pulse_engr = 1000000;  


static int pwm_init(void)
{
    rt_err_t ret = RT_EOK;
		//rt_strncpy(pwm_name, ENGINEL_DEV_NAME, RT_NAME_MAX);
	
		pwm_dev=(struct rt_device_pwm *)rt_device_find(ENGINEL_DEV_NAME);
		if (pwm_dev == RT_NULL)
    {
        rt_kprintf("can't find %s device!\n", ENGINEL_DEV_NAME);
        return RT_ERROR;
    }
		
		/* 电调解锁 */
    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL_LEFT, period, 2000000);		//设置占空比为2ms/20ms，持续3s
		rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL_RIGHT, period, 2000000);		//设置占空比为2ms/20ms，持续3s
    /* 使能 PWM 设备的输出通道 */
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL_LEFT);
		rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL_RIGHT);
		rt_thread_mdelay(3000);
		
		rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL_LEFT, period, 1000000);		//设置占空比为1ms/20ms，持续3s
		rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL_RIGHT, period, 1000000);		//设置占空比为1ms/20ms，持续3s
		rt_thread_mdelay(3000);	
		
		
    rt_thread_t thread = rt_thread_create("engine", engine_thread_entry, RT_NULL, 2048, 7, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }
    return ret;
}
INIT_APP_EXPORT(pwm_init);



static void engine_thread_entry(void *parameter)
{
	rt_uint8_t i = 0;;
	while(1)
	{
		for(i=0;i<100;i++)
		{
			pulse_engl+=10000;
			pulse_engr+=10000;
			rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL_LEFT, period, pulse_engl);
			rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL_RIGHT, period, pulse_engr);
			rt_thread_mdelay(20);
		}
				for(i=0;i<100;i++)
		{
			pulse_engl-=10000;
			pulse_engr-=10000;
			rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL_LEFT, period, pulse_engl);
			rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL_RIGHT, period, pulse_engr);
			rt_thread_mdelay(20);
		}
			rt_thread_mdelay(500);
		
	}
}


