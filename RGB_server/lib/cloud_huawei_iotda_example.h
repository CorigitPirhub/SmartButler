#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "ohos_init.h"
#include "hal_bsp_wifi.h"
#include "hal_bsp_mqtt.h"
#include "hal_bsp_log.h"
#include "cJSON.h"

#include "../lib/shared.h"

// 1.填写实例的MQTT端口号
#define SERVER_IP_PORT                      1883   // 华为云平台的IP端口号
// 1.执行ping -4 b521facbdc.st1.iotda-device.cn-north-4.myhuaweicloud.com（华为云MQTT的接入地址IPv4）
#define SERVER_IP_ADDR                      "117.78.5.125"   // 华为云平台的IP地址
// 2.创建“实例-产品-设备”，填写设备ID
#define DEVICE_ID                           "668256766bc31504f0716de2_hi3861"   // 设备ID
// 3.进入https://iot-tool.obs-website.cn-north-4.myhuaweicloud.com/，根据设备ID和密码生成MQTT客户端ID（不校验时间戳）
#define MQTT_CLIENT_ID                      "668256766bc31504f0716de2_hi3861_0_0_2024070708"   // MQTT客户端ID
// 3.同上，生成用户名MQTT
#define MQTT_USER_NAME                      "668256766bc31504f0716de2_hi3861"   // MQTT用户名
// 3.同上，生成MQTT密码
#define MQTT_PASS_WORD                      "55699ebdc9c6d84fef3a9976753e306dd1d2c72efb617f7745fbc851982d13a7"   // MQTT密码
// 4.进入产品，自定义模型，模型的服务ID应与此处的service_id的值保持一致。产品的自定义模型可以添加各种属性，对应此处的properties
#define MQTT_PAYLOAD_PUB                    "{\"services\":[{\"service_id\":\"attribute\",\"properties\":{\"RGB_LED\":%d,\"RGBHost\":\"%s\"}}]}"   // 设备信息

// 订阅接收控制命令的主题
#define MQTT_TOPIC_SUB_COMMANDS             "$oc/devices/%s/sys/commands/#"   
// 发布成功接收到控制命令后的主题
#define MQTT_TOPIC_PUB_COMMANDS_REQ         "$oc/devices/%s/sys/commands/response/request_id=%s"    
#define MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ  "$oc/devices//sys/commands/response/request_id="    
// 发布设备属性数据的主题
#define MQTT_TOPIC_PUB_PROPERTIES           "$oc/devices/%s/sys/properties/report"   
#define MALLOC_MQTT_TOPIC_PUB_PROPERTIES    "$oc/devices//sys/properties/report"   

// 定义消息队列对象的个数
#define MsgQueueObjectNumber 16   
typedef struct message_sensorData
{
  int RGB_LED;  // RGB灯的数值
  // 代表字符串
  char RGBHost[50]; // RGB灯的IP地址
}msg_sensorData_t;
msg_sensorData_t sensorData = {0};    // 传感器的数据

osThreadId_t mqtt_send_task_id; // mqtt 发布数据任务ID
osThreadId_t mqtt_recv_task_id; // mqtt 接收数据任务ID

/**
 * @brief MQTT  发布消息任务
 */
void mqtt_send_task(void *argument)
{
  // uint8_t displayBuffer[50] = {0};
  uint8_t payloadBuffer[500] = {0};
  while(1)
  {
    // 更新数值

    sensorData.RGB_LED = light_type;
    strcpy(sensorData.RGBHost, RGB_HOST);

    // 打印结果
    console_log_info("RGB_LED:%d", 
      sensorData.RGB_LED
    );
    console_log_info("RGBHost:%s", 
      sensorData.RGBHost
    );
    // 上传华为云
    char *publish_topic = (char *)malloc(strlen(MALLOC_MQTT_TOPIC_PUB_PROPERTIES) + strlen(DEVICE_ID) + 1);
    if(publish_topic != NULL)
    {
      memset(publish_topic, 0, strlen(DEVICE_ID) + strlen(MALLOC_MQTT_TOPIC_PUB_PROPERTIES) + 1);
      sprintf(publish_topic, MQTT_TOPIC_PUB_PROPERTIES, DEVICE_ID);

      memset(payloadBuffer, 0, sizeof(payloadBuffer));
      sprintf(payloadBuffer, MQTT_PAYLOAD_PUB, 
        sensorData.RGB_LED, 
        sensorData.RGBHost
      );
      MQTTClient_pub(publish_topic, payloadBuffer, strlen((char *)payloadBuffer));

      free(publish_topic);
      publish_topic = NULL;
    }
    else{
      publish_topic = NULL;
    }
    sleep(3);
  }
}

/**
 * @brief MQTT接收数据的回调函数
 */
int8_t mqttClient_sub_callback(unsigned char *topic, unsigned char *payload)
{
  if ((topic == NULL) || (payload == NULL))
    return -1;
  else 
  {
    console_log_info("topic: %s", topic);
    console_log_info("payload: %s", payload);

    // 提取出topic中的request_id
    char request_id[50] = {0};
    int ret_code = 1;   // 1为失败
    strcpy(request_id, topic + strlen(DEVICE_ID) + strlen("$oc/devices//sys/commands/request_id="));
    console_log_info("request_id: %s", request_id);
    
#if 1
    cJSON *root = NULL;
    cJSON *command_name = NULL;
    cJSON *paras = NULL;
    cJSON *value = NULL;

    root = cJSON_Parse((const char *)payload);
    if (root)// 解析JSON数据
    {
      command_name = cJSON_GetObjectItem(root, "command_name");
      paras = cJSON_GetObjectItem(root, "paras");
      if (command_name)
      {
        if (!strcmp(command_name->valuestring, "RGB_LED"))
        {
          value = cJSON_GetObjectItem(paras, "value");
          if (!strcmp(value->valuestring, "0"))
          {
            console_log_info("RGB red");
            sensorData.RGB_LED = 0;
            light_type = 0;
            ret_code = 0;   // 0为成功
          }
          else if (!strcmp(value->valuestring, "1"))
          {
            console_log_info("RGB orange");
            sensorData.RGB_LED = 1;
            light_type = 1;
            ret_code = 0;   // 0为成功
          }
          else if (!strcmp(value->valuestring, "2"))
          {
            console_log_info("RGB green");
            sensorData.RGB_LED = 2;
            light_type = 2;
            ret_code = 0;   // 0为成功
          }
          else if (!strcmp(value->valuestring, "-1")){
            console_log_info("RGB blue");
            sensorData.RGB_LED = 3;
            light_type = -1;
            ret_code = 0;   // 0为成功
          }
        }
      }

      // 向云端发送命令设置的返回值
      char *request_topic = (char*)malloc(strlen(MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ) + strlen(DEVICE_ID) + strlen(request_id) + 10);
      if(request_topic != NULL)
      {
        memset(request_topic, 0, strlen(DEVICE_ID) + strlen(MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ) + 10);
        sprintf(request_topic, MQTT_TOPIC_PUB_COMMANDS_REQ, DEVICE_ID, request_id);
        printf("topic: %s", request_topic);
        if(ret_code == 0){
          MQTTClient_pub(request_topic, "{\"result_code\":0}", strlen("{\"result_code\":0}"));
          console_log_info("data: %s", "{\"result_code\":0}");
        }
        else if(ret_code == 1){
          MQTTClient_pub(request_topic, "{\"result_code\":1}", strlen("{\"result_code\":1}"));
          console_log_info("data: %s", "{\"result_code\":1}");
        }
        free(request_topic);
      }
    }

    root = NULL;
    command_name = NULL;
    paras = NULL;
    value = NULL;
#endif
  }
    
  return 0;
}

/**
 * @brief MQTT  接收消息任务
 */
void mqtt_recv_task(void *argument)
{
  while (1)
  {
    MQTTClient_sub();
    sleep(1);
  }
}
