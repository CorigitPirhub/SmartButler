# SmartButler
# 小智管家 - 智能家居管理系统

小智管家是一款基于润和满天星系列Pegasus智能家居开发套件的智能家居管理项目。该项目通过TCP/UDP协议构建家庭局域网络，并使用红外遥控实现对各个设备的精确控制。设备与华为云IoTDA平台建立连接，实现IP地址等信息的共享，从而构建起多个开发板之间的家庭交互网络。

## 主要组成部分

项目包括以下几个主要部分：

### client文件夹（Hi3861客户端）

- 包含Hi3861开发板作为客户端的核心代码。
- 集成温湿度传感器，用于读取环境数据。
- 负责接收用户通过红外遥控发送的指令，并与其他服务器进行通信，以控制相应的智能家居设备。

### RGB_server文件夹（红、黄、绿三色灯服务器）

- 包含控制红、黄、绿三色灯服务器的代码。
- 服务器代码能够接收来自客户端的指令，并根据这些指令控制灯的颜色和状态。

### DIM_server文件夹（亮度可调节灯的服务器）

- 包含控制亮度可调节灯的代码。
- 服务器能够根据客户端的指令调整灯光的亮度，为用户提供更加个性化的照明体验。

### Door_server文件夹（门窗控制的服务器）

- 包含与门相关的控制代码，如门锁控制。
- 服务器能够接收指令，控制门的开关或者门锁的状态。

## 功能特点

- 所有设备连接WiFi，搭建服务器，并与华为云IoTDA平台建立连接，实现设备之间的自动发现和通信。
- 使用TCP/UDP协议在局域网内进行高效通信。
- 设备间通过红外遥控进行控制，LED显示屏有多个可视化界面，实时展示与用户的交互进程。
- 集成高温（火灾）、外来人员靠近等意外情况的报警系统，实时对用户发出提醒，保障家庭安全。
- 配套开发的鸿蒙南向APP，实现对Hi3861设备的远程操控和管理。

## 许可证

本项目采用[Apache 2.0许可证](#)。请参考项目内的`LICENSE`文件了解详细信息。

---

感谢使用小智管家！
