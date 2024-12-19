// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file ClockPubSubTypes.h
 * This header file contains the declaration of the serialization functions.
 *
 * This file was generated by the tool fastcdrgen.
 */

void nack_24c16()
{
    sda_24c16 = 1;    // 将数据信号线（SDA）拉高，准备开始发送非应答信号的起始状态
    delay_3us();
    delay_3us();
    scl_24c16 = 1;    // 将时钟信号线（SCL）拉高，在时钟高电平期间完成信号的传输确认
    delay_3us();
    delay_3us();
    scl_24c16 = 0;    // 拉低时钟信号线，准备下一个操作或者结束本次信号发送相关操作
    sda_24c16 = 0;    // 最后拉低数据信号线，恢复到初始状态或者符合后续通信要求的状态
}
//*************************************************
// 功能：发送IIC停止信号
// 该函数实现向24C16发送IIC通信的停止信号，按照IIC协议规范，
// 先拉低SDA线，再拉高SCL线，延时后拉高SDA线，最后拉低SCL线来表示通信结束。
void stop_24c16()
{  
    sda_24c16 = 0;    // 首先拉低数据信号线（SDA），这是发送停止信号的起始操作
    scl_24c16 = 1;    // 拉高时钟信号线（SCL），为后续SDA线状态改变及信号确认做准备
    delay_3us();
    delay_3us();                        
    sda_24c16 = 1;    // 拉高数据信号线，这是停止信号的关键标志，表示通信停止
    delay_3us();
    delay_3us();                                 
    scl_24c16 = 0;    // 拉低时钟信号线，完成停止信号的发送，恢复相关信号线状态
} 
//*************************************************
// 功能：发送启动通讯的信号
// 此函数用于向24C16发起IIC通信的启动信号，按照协议，
// 先将SDA和SCL线都拉高，延时后拉低SDA线，再延时，最后拉低SCL线开启通信。
void star_24c16()
{
    sda_24c16 = 1;    // 将数据信号线（SDA）拉高，初始状态准备
    scl_24c16 = 1;    // 将时钟信号线（SCL）拉高，与SDA配合完成启动信号的起始部分
    delay_3us();
    delay_3us();
    sda_24c16 = 0;    // 拉低数据信号线，这是启动信号的关键操作，表示通信开始
    delay_3us();
    delay_3us();
    scl_24c16 = 0;    // 拉低时钟信号线，完成启动信号的发送，为后续数据传输准备
}  
//****************************************************
// 功能：判断应答或非应答
// 说明：通讯出错时标志为1，否则为0。该函数通过检查SDA线在特定时钟状态下的电平情况，
// 来判断从24C16设备返回的是应答（ACK）还是非应答（NACK）信号，若SDA为高电平（通常表示非应答），
// 则设置错误标志flag12为1，否则清除错误标志（设为0）。
void cack_24c16()
{  
    scl_24c16 = 0;    // 先拉低时钟信号线，准备设置SDA线为输入状态并进行后续判断操作
    sda_24c16 = 1;    // 将数据信号线（SDA）拉高，准备读取其电平来判断应答情况（此时SDA作为输入）
    delay_3us();         // 短暂延时，确保信号稳定
    scl_24c16 = 1;              // 拉高时钟信号线，在时钟高电平期间读取SDA电平以判断应答情况
    flag12 = 0;    // 清除错误标志，假设初始为正常应答情况
    if (sda_24c16) flag12 = 1;    // 如果SDA线为高电平，说明可能出现通信错误，设置错误标志为1
    scl_24c16 = 0;    // 拉低时钟信号线，完成本次应答判断操作
}      
//****************************************************
// 功能：发送应答信号
// 该函数用于向24C16设备发送应答信号（ACK），按照IIC协议要求，
// 先拉低SDA线，然后拉高SCL线，延时后拉低SCL线，最后拉高SDA线恢复状态。
void mack_24c16()  
{ 
    sda_24c16 = 0;    // 拉低数据信号线（SDA），这是发送应答信号的关键操作，表示正确接收数据等情况
    scl_24c16 = 1;    // 拉高时钟信号线（SCL），在时钟高电平期间完成应答信号的传输确认
    delay_3us();
    delay_3us();
    scl_24c16 = 0;    // 拉低时钟信号线，准备下一个操作或者结束本次应答信号发送相关操作
    sda_24c16 = 1;    // 最后拉高数据信号线，恢复到初始状态或者符合后续通信要求的状态
}
//*************************************************
// 功能：向24C16写入一字节的数据
// 参数byte1为要写入的字节数据。函数通过循环8次，按照从高位到低位的顺序，
// 根据每一位的值来设置SDA线电平，然后在SCL线的上升沿将数据位发送出去，每发送一位后将数据左移一位。
void w1byte_24c16(uchar byte1)
{
    uchar i = 8;
    while (i--)    // 循环8次，因为一个字节有8位，每次处理一位数据
    {             
        delay_3us();
        delay_3us();
        delay_3us();
        if (byte1 & 0x80)    // 判断要发送的字节数据的最高位是否为1
        {
            sda_24c16 = 1;    // 如果最高位为1，则将数据信号线（SDA）拉高
        }
        else
        {
            sda_24c16 = 0;    // 如果最高位为0，则将数据信号线（SDA）拉低
        }
        delay_3us();
        delay_3us();
        delay_3us();
        scl_24c16 = 1;    // 拉高时钟信号线（SCL），在时钟上升沿将当前数据位发送出去
        delay_3us();
        delay_3us();
        delay_3us();
        scl_24c16 = 0;    // 拉低时钟信号线，准备发送下一位数据
        byte1 <<= 1;    // 将待发送的数据左移一位，准备发送下一位（高位在前依次发送）
    }
}
//****************************************************
// 功能：从24C16中读出一字节的数据
// 函数通过循环8次，每次在SCL线的上升沿读取SDA线电平，若为高电平则将读出的数据相应位置1，
// 最后返回读出的一字节数据。读出的数据存放在函数返回值中，返回的数据类型为uchar（无符号字符型）。
uchar rd1byte_24c16(void)    // 读１字节子程序(通用）读出的数据存放在返回值中
{ 
    uchar i;
    uchar ddata = 0;
    sda_24c16 = 1;    // 置IO口（这里的SDA作为输入）为1，准备读入数据
    for (i = 0; i < 8; i++)            // 循环8次，逐位读取数据，因为一个字节有8位
    {  
        ddata <<= 1;    // 将已读取的数据左移一位，为读取下一位数据腾出最低位
        delay_3us();
        delay_3us();
        delay_3us();
        scl_24c16 = 1;    // 拉高时钟信号线（SCL），在时钟高电平期间读取SDA线电平获取数据位
        if (sda_24c16) ddata++;    // 如果SDA线为高电平，则将读出的数据对应位置1（通过自增操作实现）
        delay_3us();
        delay_3us();
        delay_3us();    
        scl_24c16 = 0;    // 拉低时钟信号线，准备读取下一位数据
    }
    return ddata;    // 返回读出的一字节数据
}  
