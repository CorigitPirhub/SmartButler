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
#include "hi_io.h"
#include "iot_pwm.h"
#include "iot_errno.h"

#include "../lib/shared.h"
#define RED_LED_PIN_NAME        10
#define GREEN_LED_PIN_NAME      11
#define BLUE_LED_PIN_NAME       12

#define LED_PWM_FUNCTION        5
#define IOT_PWM_PORT_PWM1       1
#define IOT_PWM_PORT_PWM2       2
#define IOT_PWM_PORT_PWM3       3

#define LED_DELAY_TIME_US       300000
#define LED_BRIGHT              IOT_GPIO_VALUE1
#define LED_DARK                IOT_GPIO_VALUE0

#define NUM_BLINKS              2

#define ADC_RESOLUTION          4096
#define PWM_FREQ_DIVITION       64000

int light_value = 0;

static void CorlorfulLightTask(void *arg)
{
    (void)arg;
    static const unsigned int pins[] = {RED_LED_PIN_NAME, GREEN_LED_PIN_NAME, BLUE_LED_PIN_NAME};

    for (unsigned j = 0; j < sizeof(pins)/sizeof(pins[0]); j++) {
        IoTGpioSetOutputVal(pins[j], LED_BRIGHT);
        usleep(LED_DELAY_TIME_US);
        IoTGpioSetOutputVal(pins[j], LED_DARK);
        usleep(LED_DELAY_TIME_US);
    }

    hi_io_set_func(RED_LED_PIN_NAME, LED_PWM_FUNCTION);
    hi_io_set_func(GREEN_LED_PIN_NAME, LED_PWM_FUNCTION);
    hi_io_set_func(BLUE_LED_PIN_NAME, LED_PWM_FUNCTION);

    IoTPwmInit(IOT_PWM_PORT_PWM1);
    IoTPwmInit(IOT_PWM_PORT_PWM2);
    IoTPwmInit(IOT_PWM_PORT_PWM3);

    uint32_t flashDelay = 250000;
    while (1) {
        IoTPwmStart(3, light_value, PWM_FREQ_DIVITION);
        usleep(flashDelay);
        IoTPwmStop(3);
    }
}

static void ColorfulLightDemo(void)
{
    osThreadAttr_t attr;

    IoTGpioInit(RED_LED_PIN_NAME);
    IoTGpioInit(GREEN_LED_PIN_NAME);
    IoTGpioInit(BLUE_LED_PIN_NAME);

    // set Red/Green/Blue LED pin as output
    IoTGpioSetDir(RED_LED_PIN_NAME, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(GREEN_LED_PIN_NAME, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(BLUE_LED_PIN_NAME, IOT_GPIO_DIR_OUT);

    attr.name = "CorlorfulLightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(CorlorfulLightTask, NULL, &attr) == NULL) {
        printf("[ColorfulLightDemo] Falied to create CorlorfulLightTask!\n");
    }
}

APP_FEATURE_INIT(ColorfulLightDemo);
