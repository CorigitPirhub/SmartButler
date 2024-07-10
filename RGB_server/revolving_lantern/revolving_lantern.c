/*
 * Copyright (C) 2021 HiHope Open Source Organization .
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_watchdog.h"
#include "iot_pwm.h"
#include "hi_io.h"
#include "hi_adc.h"
#include "hi_errno.h"

#include "../lib/shared.h"

static int g_ledStates[3] = {0, 0, 0};
static int g_currentBright = 0;
static int g_beepState = 0;

#define     IOT_GPIO_IDX_10    10
#define     IOT_GPIO_IDX_11    11
#define     IOT_GPIO_IDX_12    12
#define     IOT_GPIO_IDX_13    13
#define     IOT_GPIO_IDX_8     8

#define     LED_TASK_STACK_SIZE 512
#define     LED_TASK_PRIO       25
#define     LED_TEST_GPIO       9
#define     IOT_GPIO_KEY        5

int light_type = -1;

static void *TrafficLightTask(const char *arg)
{
    (void)arg;

    printf("TrafficLightTask start!\r\n");
    unsigned int  pins[3] = {IOT_GPIO_IDX_10, IOT_GPIO_IDX_12, IOT_GPIO_IDX_11};

    IoTGpioInit(IOT_GPIO_KEY);
    hi_io_set_func(IOT_GPIO_KEY, HI_IO_FUNC_GPIO_5_GPIO);
    IoTGpioSetDir(IOT_GPIO_KEY, IOT_GPIO_DIR_IN);
    hi_io_set_pull(IOT_GPIO_KEY, HI_IO_PULL_UP);

    while(1) {
        // 如果light_type为-1，则表示灯全灭
        if(light_type == -1){
            for(int i = 0; i < 3; i++){
                IoTGpioSetOutputVal(pins[i], IOT_GPIO_VALUE0);
            }
            continue;
        }
        // 如果light_type为3，则表示灯全亮
        else if(light_type == 3){
            for(int i = 0; i < 3; i++){
                IoTGpioSetOutputVal(pins[i], IOT_GPIO_VALUE1);
            }
            continue;
        }
        // 如果light_type为0，则表示红灯亮
        // 如果light_type为1，则表示橙灯亮
        // 如果light_type为2，则表示绿灯亮
        else{
            for(int i = 0; i < 3; i++){
                if(i != light_type){
                    IoTGpioSetOutputVal(pins[i], IOT_GPIO_VALUE0);
                }
            }
        }
        IoTGpioSetOutputVal(pins[light_type], IOT_GPIO_VALUE1);
        usleep(25000);
        IoTGpioSetOutputVal(pins[light_type], IOT_GPIO_VALUE0);
    }
    

    return NULL;
}

static void StartTrafficLightTask(void)
{
    osThreadAttr_t attr;
    IoTGpioInit(LED_TEST_GPIO);
    IoTGpioSetDir(LED_TEST_GPIO, IOT_GPIO_DIR_OUT);

    IoTGpioInit(IOT_GPIO_IDX_10);
    IoTGpioSetDir(IOT_GPIO_IDX_10, IOT_GPIO_DIR_OUT);

    IoTGpioInit(IOT_GPIO_IDX_11);
    IoTGpioSetDir(IOT_GPIO_IDX_11, IOT_GPIO_DIR_OUT);

    IoTGpioInit(IOT_GPIO_IDX_12);
    IoTGpioSetDir(IOT_GPIO_IDX_12, IOT_GPIO_DIR_OUT);

    IoTGpioInit(IOT_GPIO_IDX_8);
    hi_io_set_func(IOT_GPIO_IDX_8, 0);
    IoTGpioSetDir(IOT_GPIO_IDX_8, IOT_GPIO_DIR_IN);
    hi_io_set_pull(IOT_GPIO_IDX_8, 1);

    IoTWatchDogDisable();

    attr.name = "TrafficLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)TrafficLightTask, NULL, &attr) == NULL) {
        printf("[LedExample] Falied to create TrafficLightTask!\n");
    }
}

APP_FEATURE_INIT(StartTrafficLightTask);