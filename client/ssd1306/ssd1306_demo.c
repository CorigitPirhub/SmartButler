#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

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

// 光标
static char cursor[3] = {">>"};
// 光标位置
int cursor_x = 14;
int cursor_y = 15;
// 表示当前所处界面
int page = 0;
// 主界面的显示内容
static char home[5][5] = {"Temp","Door","RGB","Dim"};
/**
 * @brief 主界面
 */
void home_page_task(void){
    // 光标移动逻辑
    if (HX_VALUE == 'L' && cursor_x > 14){
        HX_VALUE = 'X';
        cursor_x -= 56;
    }
    else if (HX_VALUE == 'R' && cursor_x < 70){
        HX_VALUE = 'X';
        cursor_x += 56;
    }
    else if (HX_VALUE == 'U' && cursor_y > 15){
        HX_VALUE = 'X';
        cursor_y -= 27;
    }
    else if (HX_VALUE == 'D' && cursor_y < 42){
        HX_VALUE = 'X';
        cursor_y += 27;
    }
    // 根据是否按下OK键决定page的值
    if(HX_VALUE == 'K'){
        HX_VALUE = 'X';
        if(cursor_x == 14 && cursor_y == 15){
            page = 1;   // 温湿度界面
            cursor_x = 0;
            cursor_y = 20;
        }
        else if(cursor_x == 14 && cursor_y == 42){
            page = 3;   // 三色灯界面
            cursor_x = 14;
            cursor_y = 35;
        }
        else if(cursor_x == 70 && cursor_y == 15){
            page = 2;   // 门窗控制界面
            cursor_x = 14;
            cursor_y = 35;
        }
        else if(cursor_x == 70 && cursor_y == 42){
            page = 4;   // 强度灯界面
            cursor_x = 0;
            cursor_y = 35;
        }
    }
    // 显示光标
    ssd1306_SetCursor(cursor_x, cursor_y);
    ssd1306_DrawString(cursor, Font_7x10, White);
    // 显示Temp代表温湿度界面
    ssd1306_SetCursor(28, 15);
    ssd1306_DrawString(home[0], Font_7x10, White);
    // 显示Door代表门窗控制界面
    ssd1306_SetCursor(84, 15);
    ssd1306_DrawString(home[1], Font_7x10, White);
    // 显示RGB代表三色灯界面
    ssd1306_SetCursor(28, 42);
    ssd1306_DrawString(home[2], Font_7x10, White);
    // 显示Dim代表强度灯界面
    ssd1306_SetCursor(84, 42);
    ssd1306_DrawString(home[3], Font_7x10, White);
}

/**
 * @brief 温湿度界面
 */
void Temp_page_task(void){
    // 光标移动逻辑
    if(HX_VALUE == 'D' && cursor_y == 20){
        HX_VALUE = 'X';
        cursor_y += 15;
    }
    else if(HX_VALUE == 'D' && cursor_y == 35){
        HX_VALUE = 'X';
        cursor_x = 86;
        cursor_y = 52;
    }
    else if(HX_VALUE == 'U' && cursor_y == 35){
        HX_VALUE = 'X';
        cursor_y -= 15;
    }
    else if(HX_VALUE == 'U' && cursor_y == 52){
        HX_VALUE = 'X';
        cursor_x = 0;
        cursor_y = 35;
    }
    // 根据是否按下OK键决定page的值
    if(HX_VALUE == 'K'){
        HX_VALUE = 'X';
        if(cursor_x == 86 && cursor_y == 52){
            page = 0;
            cursor_x = 14;
            cursor_y = 15;
        }
    }
    // 显示光标
    ssd1306_SetCursor(cursor_x, cursor_y);
    ssd1306_DrawString(cursor, Font_7x10, White);
    // 显示IP地址
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("IP:", Font_7x10, White);
    ssd1306_SetCursor(strlen("IP:")*7, 0);
    ssd1306_DrawString(CLIENT_HOST, Font_7x10, White);
    
    char str_temp[20];
    char str_humi[20];
    
    // 显示当前温度
    ssd1306_SetCursor(14, 20);
    ssd1306_DrawString("Temp Now:", Font_7x10, White);
    ssd1306_SetCursor(strlen("Temp Now:")*7 + 14, 20);
    sprintf(str_temp, "%.2f", global_temp);   // 将浮点数转化为字符串，从而可以在屏幕上显示
    ssd1306_DrawString(str_temp, Font_7x10, White);
    // 显示当前湿度
    ssd1306_SetCursor(14, 35);
    ssd1306_DrawString("Humi Now:", Font_7x10, White);
    ssd1306_SetCursor(strlen("Humi Now:")*7 + 14, 35);
    sprintf(str_humi, "%.2f", global_humi);
    ssd1306_DrawString(str_humi, Font_7x10, White);

    // 返回
    ssd1306_SetCursor(100, 52);
    ssd1306_DrawString("Back", Font_7x10, White);
}

/**
 * @brief 门窗控制界面
 */
void Door_page_task(void){
    // 光标移动逻辑
    if(HX_VALUE == 'D' && cursor_y == 35){
        HX_VALUE = 'X';
        cursor_x = 86;
        cursor_y = 52;
    }
    else if(HX_VALUE == 'U' && cursor_y == 52){
        HX_VALUE = 'X';
        cursor_x = 0;
        cursor_y = 35;
    }
    else if(HX_VALUE == 'R' && cursor_x == 14){
        HX_VALUE = 'X';
        cursor_x = 70;
    }
    else if(HX_VALUE == 'L' && cursor_x == 70){
        HX_VALUE = 'X';
        cursor_x = 14;
    }
    // 根据是否按下OK键决定page的值
    if(HX_VALUE == 'K'){
        HX_VALUE = 'X';
        if(cursor_x == 86 && cursor_y == 52){
            page = 0;
            cursor_x = 14;
            cursor_y = 15;
        }
        else if(cursor_x == 14){
            strcpy(request, "ondoor");
            UdpClientTest(DOOR_HOST,5678);
        }
        else if(cursor_x == 70){
            strcpy(request, "onwindow");
            UdpClientTest(DOOR_HOST,5678);
        }
    }
    // 关闭门窗
    if(HX_VALUE == '#'){
        HX_VALUE = 'X';
        strcpy(request, "off");
        UdpClientTest(DOOR_HOST,5678);
    }
    // 显示光标
    ssd1306_SetCursor(cursor_x, cursor_y);
    ssd1306_DrawString(cursor, Font_7x10, White);
    // 显示IP地址
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("IP:", Font_7x10, White);
    ssd1306_SetCursor(strlen("IP:")*7, 0);
    ssd1306_DrawString(DOOR_HOST, Font_7x10, White);
    // 选择门窗
    ssd1306_SetCursor(28, 35);
    ssd1306_DrawString("Door", Font_7x10, White);   // R代表红色
    ssd1306_SetCursor(84, 35);
    ssd1306_DrawString("Win", Font_7x10, White);   // R代表红色
    // 返回
    ssd1306_SetCursor(100, 52);
    ssd1306_DrawString("Back", Font_7x10, White);
}

/**
 * @brief 三色灯界面
 */
void RGB_page_task(void){
    // 光标移动逻辑
    if(HX_VALUE == 'R' && cursor_x < 84){
        HX_VALUE = 'X';
        cursor_x += 35;
    }
    else if(HX_VALUE == 'L' && cursor_x > 14){
        HX_VALUE = 'X';
        cursor_x -= 35;
    }
    else if(HX_VALUE == 'D' && cursor_y == 35){
        HX_VALUE = 'X';
        cursor_x = 86;
        cursor_y = 52;
    }
    else if(HX_VALUE == 'U' && cursor_y == 52){
        HX_VALUE = 'X';
        cursor_x = 14;
        cursor_y = 35;
    }
    // 根据是否按下OK键决定page的值
    if(HX_VALUE == 'K'){
        HX_VALUE = 'X';
        if(cursor_x == 86 && cursor_y == 52){
            page = 0;
            cursor_x = 14;
            cursor_y = 15;
        }
        else if(cursor_x == 14){
            strcpy(request, "red");
            UdpClientTest(RGB_HOST,5678);
        }
        else if(cursor_x ==49){
            strcpy(request, "orange");
            UdpClientTest(RGB_HOST,5678);
        }
        else if(cursor_x == 84){
            strcpy(request, "green");
            UdpClientTest(RGB_HOST,5678);
        }
    }
    // 关闭灯光
    if(HX_VALUE == '#'){
        HX_VALUE = 'X';
        strcpy(request, "off");
        UdpClientTest(RGB_HOST,5678);
    }
    // 显示光标
    ssd1306_SetCursor(cursor_x, cursor_y);
    ssd1306_DrawString(cursor, Font_7x10, White);
    // 显示IP地址
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("IP:", Font_7x10, White);
    ssd1306_SetCursor(strlen("IP:")*7, 0);
    ssd1306_DrawString(RGB_HOST, Font_7x10, White);

    // 选择颜色
    ssd1306_SetCursor(0, 20);
    ssd1306_DrawString("Color:", Font_7x10, White);
    ssd1306_SetCursor(28, 35);
    ssd1306_DrawString("R", Font_7x10, White);   // R代表红色
    ssd1306_SetCursor(63, 35);
    ssd1306_DrawString("O", Font_7x10, White);   // O代表红色
    ssd1306_SetCursor(98, 35);
    ssd1306_DrawString("G", Font_7x10, White);   // R代表红色
    // 返回
    ssd1306_SetCursor(100, 52);
    ssd1306_DrawString("Back", Font_7x10, White);
}

/**
 * @brief 强度灯界面
 */
char digit[10] = "";
void Dim_page_task(void){
    // 光标移动逻辑
    if(HX_VALUE == 'D' && cursor_y == 35){
        HX_VALUE = 'X';
        cursor_x = 86;
        cursor_y = 52;
    }
    else if(HX_VALUE == 'U' && cursor_y == 52){
        HX_VALUE = 'X';
        cursor_x = 0;
        cursor_y = 35;
    }
    // 根据是否按下OK键决定page的值
    if(HX_VALUE == 'K'){
        HX_VALUE = 'X';
        if(cursor_y == 35 && strcmp(digit, "") != 0){
            // strcpy(request, digit);
            // UdpClientTest(DIM_HOST,5678);
            memset(digit, 0, sizeof(digit));
        }
        else if(cursor_y == 52){
            page = 0;
            cursor_x = 14;
            cursor_y = 15;
        }
    }
    // 输入数字
    if(isdigit(HX_VALUE) && cursor_y == 35 && strlen(digit) <= 2){
        digit[strlen(digit)] = HX_VALUE;
        HX_VALUE = 'X';
    }
    else if(HX_VALUE != 'X'){
        memset(digit, 0, sizeof(digit));
    }
    // 关闭灯光
    if(HX_VALUE == '#'){
        HX_VALUE = 'X';
        strcpy(request, "off");
        UdpClientTest(DIM_HOST,5678);
    }
    // 显示光标
    ssd1306_SetCursor(cursor_x, cursor_y);
    ssd1306_DrawString(cursor, Font_7x10, White);
    // 显示IP地址
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("IP:", Font_7x10, White);
    ssd1306_SetCursor(strlen("IP:")*7, 0);
    ssd1306_DrawString(DIM_HOST, Font_7x10, White);
    // 输入亮度
    ssd1306_SetCursor(14, 20);
    ssd1306_DrawString("(0-99)", Font_7x10, White);
    ssd1306_SetCursor(14, 35);
    ssd1306_DrawString("Dim:", Font_7x10, White);
    ssd1306_SetCursor(strlen("Dim:")*7 + 14, 35);
    ssd1306_DrawString(digit, Font_7x10, White);
    // 返回
    ssd1306_SetCursor(100, 52);
    ssd1306_DrawString("Back", Font_7x10, White);
}



/**
 * @brief 主函数，显示SSD1306屏幕
 */
void ssd1306_report(void) {
    // 填充背景色
    ssd1306_Fill(Black);

    // 根据page的值决定展示的界面
    if(page == 0){
        home_page_task();   // 主界面
    }
    else if(page == 1){
        Temp_page_task();   // 温度界面
    }
    else if(page == 2){
        Door_page_task();   // 门窗界面
    }
    else if(page == 3){
        RGB_page_task();   // 三色灯界面
    }
    else if(page == 4){
        Dim_page_task();   // 强度灯界面
    }

    //刷新屏幕
    uint32_t t = 1000;
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