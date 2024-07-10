#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "hi_types_base.h" 
#include "hi_errno.h"
#include "hi_io.h"
#include "hi_gpio.h"
#include "hi_i2c.h"
#include "hi_time.h"
#include "hi_watchdog.h"
#include "hx1838.h"

#include "../lib/shared.h"
char HX_VALUE = 'X';   // 用于数据交换的接收红外码值

hi_bool hx_state = HI_FALSE;    // 端口状态
hi_gpio_value hx_vol = 0;       // 端口电压值
hi_u8 hx_data = 0;              // 接收数据

// 码值对照表
static hi_u8 code_list[17][2] = {
    {0xa2, '1'}, {0x62, '2'}, {0xe2, '3'}, 
    {0x22, '4'}, {0x02, '5'}, {0xc2, '6'}, 
    {0xe0, '7'}, {0xa8, '8'}, {0x90, '9'}, 
    {0x68, '*'}, {0x98, '0'}, {0xb0, '#'}, 
                 {0x18, 'U'}, 
    {0x10, 'L'}, {0x38, 'K'}, {0x5a, 'R'}, 
                 {0x4a, 'D'}, 
};

/**
 * @brief 查找码值
 */
char find_value(hi_u8 hx_data)
{
    for(hi_u8 i=0; i<17; i++)
    {
        if( hx_data == code_list[i][0] )
        {
            return code_list[i][1];
        }
    }
    return -1;
}
/**
 * @brief 红外接收端 中断
 */
static hi_void Hx_Isr_Callback(hi_void)
{
    hx_state = HI_TRUE;
}

/**
 * @brief 主任务
 */
static void *Task(const char *arg)
{
    (void)arg;

    // 关闭看门狗
    hi_watchdog_disable();
    // 初始化GPIO
    hi_gpio_init();
    // 初始化红外接收端
    Hx1838_Init();

    // 设置中断
    hi_gpio_register_isr_function(
        HX_GPIO_ID,    // 指定触发中断的引脚为HX_GPIO_ID
        HI_INT_TYPE_EDGE,    // 设置为下降沿触发
        HI_GPIO_EDGE_FALL_LEVEL_LOW,    // 设置为低电平触发
        Hx_Isr_Callback,    // 中断回调函数为Hx_Isr_Callback
        HI_NULL);   // 设置为无上下文

    while(1)
    {
        hi_gpio_get_input_val(HX_GPIO_ID, &hx_vol);
        if(hx_state)
        {
            hi_gpio_set_isr_mask(HX_GPIO_ID, HI_TRUE);
            hx_data = Get_Hx_Data();
            if(hx_data > 0 )
            {
                // 根据码值对照表查找码值
                char hx_value = find_value(hx_data);
                HX_VALUE = hx_value;
                printf(" HX DATA = %c \n", hx_value);
            }else{
                printf(" GET HX DATA Falied! \n");
            }
            hx_data = 0;
            hi_udelay(50 * 1000);
            hx_state = HI_FALSE;
            hi_gpio_set_isr_mask(HX_GPIO_ID, HI_FALSE);
        }
    }   
    return NULL;
}

/**
 * @brief 系统初始化
 */
static void Entry(void)
{
    // 创建Task线程
    osThreadAttr_t attr;
    attr.name = "Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    if( osThreadNew( (osThreadFunc_t)Task, NULL, &attr ) == NULL)
    {
        printf("[hx1838] Falied to create Task!\n");
    }
    else{
        printf("[hx1838] Create Task Success!\n");
    }
}
// 系统调用entry作为程序的入口
SYS_RUN(Entry);