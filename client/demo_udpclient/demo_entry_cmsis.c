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
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "net_common.h"
#include "net_demo.h"
#include "net_params.h"
#include "wifi_connecter.h"

#include "../lib/cloud_huawei_iotda_example.h"

static void NetDemoTask(void *arg)
{
    (void)arg;
    WifiDeviceConfig config = {0};

    // 准备AP的配置参数
    strcpy(config.ssid, PARAM_HOTSPOT_SSID);
    strcpy(config.preSharedKey, PARAM_HOTSPOT_PSK);
    config.securityType = PARAM_HOTSPOT_TYPE;

    osDelay(10);

    // 连接到AP
    printf("ssid = %s", config.ssid);
    int netId = ConnectToHotspot(&config);

    // 等待网络连接成功
    int timeout = 3;
    while (timeout--) {
        printf("After %d seconds, I will start %s test!\r\n", timeout, GetNetDemoName());
        osDelay(100);
    }

    // 注册MQTT接收数据的回调函数
    p_MQTTClient_sub_callback = &mqttClient_sub_callback;
    
    // 连接MQTT服务器
    if (MQTTClient_connectServer(SERVER_IP_ADDR, SERVER_IP_PORT) != 0)
    {
        console_log_error("mqttClient_connectServer failed!");
        sleep(2);
    }
    else
    {
        sleep(1);
        console_log_info("mqttClient_connectServer success!");
    }

    // 初始化MQTT客户端
    if (MQTTClient_init(MQTT_CLIENT_ID, MQTT_USER_NAME, MQTT_PASS_WORD) != 0)
    {
        console_log_error("mqttClient_init failed!");
        sleep(2);
    }
    else
    {
        sleep(1);
        console_log_info("mqttClient_init success!");
    }

    // 订阅MQTT主题
    if (MQTTClient_subscribe(MQTT_TOPIC_SUB_COMMANDS) != 0)
    {
        console_log_error("mqttClient_subscribe failed!");
        sleep(2);
    }
    else
    {
        sleep(1);
        console_log_info("mqttClient_subscribe success!");
    }

    //  创建线程
    osThreadAttr_t options;
    options.attr_bits = 0;
    options.cb_mem = NULL;
    options.cb_size = 0;
    options.stack_mem = NULL;
    options.priority = osPriorityNormal;

    // 创建发布消息任务的线程
    options.name = "mqtt_send_task";
    options.stack_size = 1024*10;
    mqtt_send_task_id = osThreadNew((osThreadFunc_t)mqtt_send_task, NULL, &options);
    if (mqtt_send_task_id != NULL)
    {
        console_log_info("ID = %d, Create mqtt_send_task_id is OK!", mqtt_send_task_id);
    }

    // 创建接收数据任务的线程
    options.name = "mqtt_recv_task";
    options.stack_size = 1024*10;
    mqtt_recv_task_id = osThreadNew((osThreadFunc_t)mqtt_recv_task, NULL, &options);
    if (mqtt_recv_task_id != NULL)
    {
        console_log_info("ID = %d, Create mqtt_recv_task_id is OK!", mqtt_recv_task_id);
    }

    // 开启服务器测试
    NetDemoTest(PARAM_SERVER_PORT, PARAM_SERVER_ADDR);

    // 断开连接
    printf("disconnect to AP ...\r\n");
    DisconnectWithHotspot(netId);
    printf("disconnect to AP done!\r\n");
}

static void NetDemoEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "NetDemoTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    if (osThreadNew(NetDemoTask, NULL, &attr) == NULL) {
        printf("[NetDemoEntry] Falied to create NetDemoTask!\n");
    }
}

SYS_RUN(NetDemoEntry);