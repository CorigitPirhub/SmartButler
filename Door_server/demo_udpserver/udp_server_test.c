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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include "net_demo.h"
#include "net_common.h"

#include "../lib/shared.h"
static char message[128] = "";
void UdpServerTest(unsigned short port)
{
    ssize_t retval = 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket

    struct sockaddr_in clientAddr = {0};
    socklen_t clientAddrLen = sizeof(clientAddr);
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (retval < 0) {
        printf("bind failed, %ld!\r\n", retval);
        goto do_cleanup;
    }
    printf("bind to port %d success!\r\n", port);

    while(1){
        memset(message, 0, sizeof(message)); // 清空消息缓冲区
        clientAddrLen = sizeof(clientAddr); // 重置客户端地址结构大小

        retval = recvfrom(sockfd, message, sizeof(message), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (retval < 0) {
            printf("recvfrom failed, %ld!\r\n", retval);
            goto do_cleanup;
        }
        printf("recv message {%s} %ld done!\r\n", message, retval);
        printf("peer info: ipaddr = %s, port = %d\r\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        if(strcmp(message, "ondoor") == 0){
            open_door = true;
        }
        else if(strcmp(message, "onwindow") == 0){
            open_window = true;
        }
        else if (strcmp(message, "off") == 0)
        {
            open_door = false;
            open_window = false;
        }
        

        retval = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
        if (retval <= 0) {
            printf("send failed, %ld!\r\n", retval);
            goto do_cleanup;
        }
        printf("send message {%s} %ld done!\r\n", message, retval);
    }

do_cleanup:
    printf("do_cleanup...\r\n");
    close(sockfd);
}

SERVER_TEST_DEMO(UdpServerTest);