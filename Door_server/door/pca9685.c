#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "hi_types_base.h" 
#include "hi_errno.h"

#include "hi_io.h"
#include "hi_gpio.h"

#include "hi_i2c.h"

#include "hi_time.h"
#include "hi_watchdog.h"

#include "pca9685.h"

// 输入1个数据
static hi_void PCA9685_I2C_Write_Data(hi_u8 addr, hi_u8 data);
// 写入1组数据
static hi_void PCA9685_I2C_Write_Datas(hi_u8 *data_send_buf, hi_u8 data_send_len);
//  读取1个数据
static hi_u8 PCA9685_I2C_Read_Data(hi_u8 addr);


// 输入1个数据
static hi_void PCA9685_I2C_Write_Data(hi_u8 addr, hi_u8 data)
{
    hi_u8 data_send_buf[2] = {addr, data};
    hi_i2c_data pca9685_i2c_data = {0};
    pca9685_i2c_data.send_buf = data_send_buf;
    pca9685_i2c_data.send_len = 2;

    if(hi_i2c_write(PCA9685_I2C_IDX, PCA9685_WRITE_ADDR, &pca9685_i2c_data) != 0)
    {
        printf(" [PCA9685_I2C_Write_Data] Failed! \n");
    }
}

// 写入1组数据
static hi_void PCA9685_I2C_Write_Datas(hi_u8 *data_send_buf, hi_u8 data_send_len)
{
    hi_i2c_data pca9685_i2c_data = {0};
    pca9685_i2c_data.send_buf = data_send_buf;
    pca9685_i2c_data.send_len = data_send_len;
    if(hi_i2c_write(PCA9685_I2C_IDX, PCA9685_WRITE_ADDR, &pca9685_i2c_data) != 0)
    {
        printf(" [PCA9685_I2C_Write_Datas] Failed! \n");
    }
}

//  读取1个数据
static hi_u8 PCA9685_I2C_Read_Data(hi_u8 addr)
{
    hi_u8 data_send_buf[1] = {addr};
    hi_u8 data_receive_buf[1] = {0};
    hi_i2c_data pca9685_i2c_data = {0};

    pca9685_i2c_data.send_buf = data_send_buf;
    pca9685_i2c_data.send_len = 1;
    pca9685_i2c_data.receive_buf = data_receive_buf;
    pca9685_i2c_data.receive_len = 1;
    
    if(hi_i2c_write(PCA9685_I2C_IDX, PCA9685_WRITE_ADDR, &pca9685_i2c_data) != 0)
    {
        printf(" [PCA9685_I2C_Read_Data] write Failed! \n");
    }    

    if(hi_i2c_read(PCA9685_I2C_IDX, PCA9685_READ_ADDR, &pca9685_i2c_data) != 0)
    {
        printf(" [PCA9685_I2C_Read_Data] read Failed! \n");
    }

    return data_receive_buf[0];
}

// 初始化
hi_void PCA9685_Init(hi_void)
{
    PCA9685_I2C_Write_Data(PCA9685_MODE1, 0x00);    // 已禁用重新启动 0000 0000 [4] = 0 正常模式 等待 500μs
    hi_udelay(500);
    PCA9685_I2C_Write_Data(PCA9685_MODE1, 0x80);    // 已启用重新启动 1000 0000 [4] = 0 正常模式 等待 500μs
    hi_udelay(500);
}


// 重置
hi_u8 PCA9685_Reset(hi_void)
{
    PCA9685_I2C_Write_Data(PCA9685_MODE1, 0x00);
    hi_udelay(500);
}


// 设置PWM频率
hi_u8 PCA9685_Set_PWM_Freq(hi_u8 freq)
{
    hi_float prescale = 25000000 / 4096 / (freq * 0.95) - 1;
    hi_u8 val = (hi_u8)(prescale + 0.5);    // 向上取整

    PCA9685_I2C_Write_Data(PCA9685_MODE1, 0x10);    // 0x10  0001 0000  [7] = 0 已禁用重新启动  [4] = 1 休眠
    PCA9685_I2C_Write_Data(PCA9685_PRE_SCALE, val);
    PCA9685_I2C_Write_Data(PCA9685_MODE1, 0xa1);    // 0xa1 1010 0001 [7] = 1 已启用重新启动 [5] = 1 启用寄存器自动增量 [0] = 1 PCA9685响应 LED All Call I2C总线地址
    hi_udelay(500);
}


// 设置PWM 占空比
hi_void PCA9685_Set_PWM(hi_u8 num, hi_u16 on, hi_u16 off)
{
    hi_u8 datas[5] = {0};

    datas[0] = PCA9685_LED0_ON_L + 4 * num;
    datas[1] = on & 0xff;
    datas[2] = (on >> 8) & 0xff;
    datas[3] = off & 0xff;
    datas[4] = (off >> 8) & 0xff;

    PCA9685_I2C_Write_Datas(datas, 5);
    hi_udelay(500);
}

// 设置 角度
hi_void PCA9685_Angle(hi_u8 num, hi_u8 ang)
{
    hi_u16 off = (hi_u16)(101 + ang * 2.28);     // (511 - 101) / 180 = 2.28  已经向上取整
    PCA9685_Set_PWM(num, 0, off);
}

