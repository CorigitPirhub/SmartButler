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
    
    // 显示当前开关门的情况
    ssd1306_SetCursor(0, 20);
    if(open_door == HI_FALSE){
        ssd1306_DrawString("Door Now:Off", Font_7x10, White);
        ssd1306_SetCursor(strlen("Door Now:Off")*7, 20);
    }
    else{
        ssd1306_DrawString("Door Now:On", Font_7x10, White);
        ssd1306_SetCursor(strlen("Door Now:On")*7, 20);
    }
    
    // 显示当前开关窗的情况
    ssd1306_SetCursor(0, 35);
    if(open_window == HI_FALSE){
        ssd1306_DrawString("Windows Now:Off", Font_7x10, White);
        ssd1306_SetCursor(strlen("Window Now:Off")*7, 35);
    }
    else if(open_window == HI_TRUE){
        ssd1306_DrawString("Windows Now:On", Font_7x10, White);
        ssd1306_SetCursor(strlen("Window Now:On")*7, 35);
    }

    // 显示IP地址
    ssd1306_SetCursor(0, 50);
    ssd1306_DrawString("IP:", Font_7x10, White);
    ssd1306_SetCursor(strlen("IP:")*7, 50);
    ssd1306_DrawString(DOOR_HOST, Font_7x10, White);

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