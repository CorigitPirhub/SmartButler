#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "iot_watchdog.h"
#include "iot_pwm.h"
#include "hi_adc.h"
#include "hi_errno.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "hi_io.h"
#include "hi_gpio.h"
#include "iot_gpio.h"
#include "hi_i2c.h"
#include "hi_time.h"
#include "pca9685.h"

#include "..\lib\shared.h"

#define SDA_IO_NAME     HI_IO_NAME_GPIO_13
#define SDA_IO_FUNC     HI_IO_FUNC_GPIO_13_I2C0_SDA

#define SCL_IO_NAME     HI_IO_NAME_GPIO_14
#define SCL_IO_FUNC     HI_IO_FUNC_GPIO_14_I2C0_SCL

#define I2C_IDX         HI_I2C_IDX_0
#define I2C_BAUDRATE    400000

#define PCA9685_PIN0    0
#define PCA9685_PIN1    1
#define PCA9685_PIN2    2
#define PCA9685_PIN3    3
#define IOT_GPIO_KEY    5


clock_t time1 ,time2;      //获取系统时间，用于执行中断
double duration;       //根据time1和time2计算时间间隔

hi_bool open_door = HI_FALSE;          // 门的开关状态
hi_bool open_window = HI_FALSE;          // 第一个窗的开关状态

void PCA9685_Controldoor(int);
void PCA9685_Controlwindows(int);

/**
 * @brief 获取电压值函数
 * 
 * @return 电压值 
 */
static float GetVoltage(void)
{
    unsigned int ret;
    unsigned short data;
    // 该函数通过使用hi_adc_read()函数来读取 HI_ADC_CHANNEL_2 的数值存储在data中， 
    // HI_ADC_EQU_MODEL_8 表示8次平均算法模式，
    // HI_ADC_CUR_BAIS_DEFAULT 表示默认的自动识别模式，
    ret = hi_adc_read((hi_adc_channel_index)HI_ADC_CHANNEL_2, &data, 
                    HI_ADC_EQU_MODEL_8, HI_ADC_CUR_BAIS_DEFAULT, 0);
    if (ret != HI_ERR_SUCCESS) {
        printf("ADC Read Fail\n");
        return;
    }
    // 最后通过 data * 1.8 * 4 / 4096.0 计算出实际的电压值。
    return (float)data * 1.8 * 4 / 4096.0;
}
// 按键中断
static void OnButtonPressed(char* arg)
{
    (void*)arg;
    float voltage;
    voltage = GetVoltage();
    //计算与上一次中断的时间间隔duration
    time1 = time2;
	time2 = time(NULL);
    duration = ((double)(time2 - time1));
    //当duration大于0.5时执行中断效果
	if(duration > 0.5){
        // 通过电压检测按下了哪个按键
        // 当电压大于0.01且小于0.4时，按下了门的开关键
        if((voltage > 0.01) && (voltage < 0.4))
        {
            if(open_door == HI_FALSE){
                open_door = HI_TRUE;
            }
            else{
                open_door = HI_FALSE;
            }
        }
        // 当电压大于0.4且小于0.8时，按下了第一个窗的开关键
        if((voltage >= 0.4) && (voltage <= 0.8))
        {
            if(open_window == HI_FALSE){
                open_window = HI_TRUE;
            }
            else{
                open_window = HI_FALSE;
            }
        }
    }
}
// I2C初始
hi_void I2C_Init(hi_void)
{
    hi_io_set_func(SDA_IO_NAME, SDA_IO_FUNC);
    hi_io_set_func(SCL_IO_NAME, SCL_IO_FUNC);

    if(hi_i2c_init(I2C_IDX, I2C_BAUDRATE) != 0)
    {
        printf(" [ I2C Init ] Failed! \n");
    }
    else{
        printf(" [ I2C Init ] Success! \n");
    }
}
/*
* 控制门
*/
void PCA9685_Controldoor(int angle)
{
    PCA9685_Angle(PCA9685_PIN0, angle);
    PCA9685_Set_PWM(PCA9685_PIN0, 0, 0);
}
/*
* 控制窗
*/
void PCA9685_Controlwindows(int angle)
{
    PCA9685_Angle(PCA9685_PIN1, angle);
    PCA9685_Set_PWM(PCA9685_PIN1, 0, 0);
}

// 主任务
static void *Task(const char *arg)
{
    printf(" [ pca9685 t5 ] \n");

    hi_gpio_init();

    // 设置中断
    IoTGpioInit(IOT_GPIO_KEY);
    hi_io_set_func(IOT_GPIO_KEY, HI_IO_FUNC_GPIO_5_GPIO);
    IoTGpioSetDir(IOT_GPIO_KEY, IOT_GPIO_DIR_IN);
    hi_io_set_pull(IOT_GPIO_KEY, HI_IO_PULL_UP);
    IoTGpioRegisterIsrFunc(IOT_GPIO_KEY, IOT_INT_TYPE_LEVEL, 
                            IOT_GPIO_EDGE_FALL_LEVEL_LOW,
                            OnButtonPressed, NULL);
    // 初始化
    I2C_Init();
    PCA9685_Init();
    PCA9685_Set_PWM_Freq(50);
    PCA9685_Set_PWM(PCA9685_PIN0, 0, 0);
    PCA9685_Set_PWM(PCA9685_PIN1, 0, 0);
    PCA9685_Set_PWM(PCA9685_PIN2, 0, 0);

    while(1)
    {   
        // 检测门的情况
        if(open_door == HI_TRUE){
            PCA9685_Controldoor(90);
        }
        else{
            PCA9685_Controldoor(0);
        }
        usleep(100000);
        // 检测窗的情况
        if(open_window == HI_TRUE){
            PCA9685_Controlwindows(90);
        }
        else{
            PCA9685_Controlwindows(0);
        }
        usleep(100000);
    }
    return NULL;
}


static void Entry(void)
{
    osThreadAttr_t attr;

    attr.name = "Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2 * 1024;
    attr.priority = 25;

    if(osThreadNew((osThreadFunc_t)Task, NULL, &attr) == NULL)
    {
        printf("Falied to create Task!\n");
    }
}

SYS_RUN(Entry);