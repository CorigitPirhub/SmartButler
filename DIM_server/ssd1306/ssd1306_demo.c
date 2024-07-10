#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_pwm.h"
#include "iot_i2c.h"
#include "iot_errno.h"

#include "ssd1306.h"
#include "hi_io.h"
#include "../lib/shared.h"

#define OLED_I2C_BAUDRATE (400*1000)


void ssd1306_report(void) {
    uint32_t t = 1000;

    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("Smart Housekeeper", Font_7x10, White);
    
    char str_inte[20];
    
    // 显示当前光照强度
    ssd1306_SetCursor(0, 20);
    sprintf(str_inte, "%.2f", global_inte);   // 将浮点数转化为字符串，从而可以在屏幕上显示
    ssd1306_DrawString("Inte now:", Font_7x10, White);
    ssd1306_SetCursor(strlen("Inte now:")*7, 20);
    ssd1306_DrawString(str_inte, Font_7x10, White);
    
    char str_light[20];
    // 显示当前湿度
    ssd1306_SetCursor(0, 35);
    ssd1306_DrawString("Light Now:", Font_7x10, White);
    ssd1306_SetCursor(strlen("Light Now:")*7, 35);
    sprintf(str_light, "%d", light_value);   // 将浮点数转化为字符串，从而可以在屏幕上显示
    ssd1306_DrawString(str_light, Font_7x10, White);

    // 显示IP地址
    ssd1306_SetCursor(0, 50);
    ssd1306_DrawString("IP:", Font_7x10, White);
    ssd1306_SetCursor(strlen("IP:")*7, 50);
    ssd1306_DrawString(DIM_HOST, Font_7x10, White);
    //刷新屏幕
    ssd1306_UpdateScreen();
    HAL_Delay(t);
}

void Ssd1306TestTask(void* arg)
{
    (void) arg;
    IoTGpioInit(HI_IO_NAME_GPIO_13);
    IoTGpioInit(HI_IO_NAME_GPIO_14);

    hi_io_set_func(HI_IO_NAME_GPIO_13, HI_IO_FUNC_GPIO_13_I2C0_SDA);
    hi_io_set_func(HI_IO_NAME_GPIO_14, HI_IO_FUNC_GPIO_14_I2C0_SCL);
    
    IoTI2cInit(0, OLED_I2C_BAUDRATE);

    // WatchDogDisable();

    usleep(20*1000);
    ssd1306_Init();
    ssd1306_Fill(Black);

    usleep(2000*1000);

    uint32_t start = HAL_GetTick();
    ssd1306_UpdateScreen();
    uint32_t end = HAL_GetTick();
    printf("ssd1306_UpdateScreen time cost: %d ms.\r\n", end - start);

    while (1) {
        ssd1306_report();
    }
}

void Ssd1306TestDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "Ssd1306Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    if (osThreadNew(Ssd1306TestTask, NULL, &attr) == NULL) {
        printf("[Ssd1306TestDemo] Falied to create Ssd1306TestTask!\n");
    }
}

APP_FEATURE_INIT(Ssd1306TestDemo);