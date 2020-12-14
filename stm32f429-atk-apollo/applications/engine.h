#ifndef __ENGINE_H__
#define __ENGINE_H__
#include <rtthread.h>
#include <rtdevice.h>
#include <rtthread.h>
#include <board.h>


#define ENGINEL_DEV_NAME        "pwm4"  /* PWM设备名称 */
#define PWM_DEV_CHANNEL_LEFT     3       //左引擎通道  PB8
#define PWM_DEV_CHANNEL_RIGHT    4       //右引擎通道 PB9



static void engine_thread_entry(void *parameter);















#endif

