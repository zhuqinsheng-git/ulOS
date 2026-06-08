/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-11     zhuqinsheng   the first version
 * 2025-8-22     zhuqinsheng   修复了一些通信时序问题
 */
#include "ul_softwarei2c.h"

/**
*******************************************************************
* @brief    
*                    __________     
*           SCL : __/          \_____ 
*                 ________          
*           SDA :         \___________ 
*******************************************************************
*/
static void i2c_start(ul_swi2c_t *self)
{ 
    self->sda_write(1);
    self->scl_write(1);
    self->delay_xxus();
    self->sda_write(0);
    self->delay_xxus();
    self->scl_write(0);
    self->delay_xxus();
}

/**
*******************************************************************
* @brief    
*                       _______________     
*          SCL : ______/          
*                __        ____________  
*          SDA:    \______/
*******************************************************************
*/
static void i2c_stop(ul_swi2c_t *self)
{
    self->sda_write(0);
    self->delay_xxus();
    self->scl_write(1);
    self->delay_xxus();
    self->sda_write(1);
}

/**
*******************************************************************
* @brief    
*                         ______
*           SCL: ________/      \______    
*                ______________________    
*           SDA: \\\___________________
*******************************************************************
*/
static void i2c_sendbyte(ul_swi2c_t *self, ul_uint8_t byte)
{                          
    for (ul_uint8_t i = 0; i < 8; i++)   // 循环8次，从高到低取出字节的8个位
	{     
        self->scl_write(0);        		  // 时钟线SCL输出低电平
        
		if ((byte & 0x80))            // 2#取出字节最高位，并判断为‘0’还是‘1’，从而做出相应的操作
		{
			self->sda_write(1);            // 数据线SDA输出高电平，数据位为‘1’
		}
		else
		{  
			self->sda_write(0);      	  // 数据线SDA输出低电平，数据位为‘0’
		}
		
		byte <<= 1;            		  // 左移一位，次高位移到最高位
		
		self->delay_xxus();          		  // 延时4us
		self->scl_write(1);                // 3#时钟线SCL输出高电平
		self->delay_xxus();          		  // 延时4us
	}  
    
    self->scl_write(0);        		  // 4#时钟线SCL输出低电平
	self->delay_xxus();                  // 延时4us  
}

/**
*******************************************************************
* @brief    
*                       ______
*           SCL: ______/      \___        
*                ____________________    
*           SDA: \\\\______________\\\
*******************************************************************
*/
static ul_uint8_t i2c_readbyte(ul_swi2c_t *self)
{
    ul_uint8_t byte = 0;           		// byte用来存放接收的数据
    
	self->sda_write(1);                      // 释放SDA
    
	for (ul_uint8_t i = 0; i < 8; i++)     // 循环8次，从高到低读取字节的8个位
	{
		self->scl_write(1);          		// 时钟线SCL输出高电平
		self->delay_xxus();            		// 延时4us
		byte <<= 1;          			// 左移一位，空出新的最低位

		if (self->sda_read())       		// 读取数据线SDA的数据位
		{
			byte |= 1;            			// 在SCL的上升沿后，数据已经稳定，因此可以取该数据，存入最低位
		}
        
		self->scl_write(0);          		// 时钟线SCL输出低电平
		self->delay_xxus();             		// 延时4us
	} 

	return byte;           				// 返回读取到的数据
}

/**
*******************************************************************
* @brief    
*                _______|____     
*           SCL:        |    \_________    
*                _______|     
*           SDA:         \_____________ 
*******************************************************************
*/
static ul_ecode i2c_waitack(ul_swi2c_t *self)
{
    ul_uint32_t errTimes = 0;  // bug，应该设置成32位，不然下面的超时设置成255以上时，如果一直没有等到应答，就会卡死死循环
	
	self->sda_write(1);             // 释放SDA总线,很重要
	self->delay_xxus();               // 延时4us
	
	self->scl_write(1);             // 时钟线SCL输出高电平
	self->delay_xxus();               // 延时4us

	while (self->sda_read())      // 读回来的数据如果是高电平，即接收端没有应答
	{
		errTimes++;            // 计数器加1

		if (errTimes > 250000)    // 如果超过250次，则判断为接收端出现故障，因此发送结束信号
		{
			i2c_stop(self);         // 产生一个停止信号
			return UL_ETIMEOUT;      // 返回值为1，表示没有收到应答信号
		}
	}

	self->scl_write(0);             // 表示已收到应答信号，时钟线SCL输出低电平
	self->delay_xxus();               // 延时4us
	
	return UL_EOK;               // 返回值为0，表示接收应答成功 
}

/**
*******************************************************************
* @brief    
*                         ______
*           SCL: ________/      \____________    
*                __                     ______
*           SDA:   \___________________/        
*******************************************************************
*/
static void i2c_sendack(ul_swi2c_t *self)
{
	self->sda_write(0);          // 2#数据线SDA输出低电平
	self->delay_xxus();            // 延时4us
	self->scl_write(1);          // 3#时钟线SCL输出高电平,在SCL上升沿前就要把SDA拉低，为应答信号
	self->delay_xxus();            // 延时4us
	self->scl_write(0);          // 4#时钟线SCL输出低电平
	self->delay_xxus();            // 延时4us
	self->sda_write(1);          // 5#数据线SDA输出高电平，释放SDA总线,很重要
}

/**
*******************************************************************
* @brief    
*                        ______
*          SCL: ________/      \______    
*               __ ___________________    
*          SDA: __/
*******************************************************************
*/
static void i2c_sendnack(ul_swi2c_t *self)
{
	self->sda_write(1);          // 2#数据线SDA输出高电平
	self->delay_xxus();            // 延时4us
	self->scl_write(1);          // 3#时钟线SCL输出高电平，在SCL上升沿前就要把SDA拉高，为非应答信号
	self->delay_xxus();            // 延时4us
	self->scl_write(0);          // 4#时钟线SCL输出低电平
	self->delay_xxus();            // 延时4us
}

/**
 * 软件i2c初始化函数
 *
 * @param self        软件I2C对象
 *
 * @return 错误代码
 */
ul_ecode ul_swi2c_init(ul_swi2c_t *self)
{
    if (self == NULL)
    {
        return UL_ENULL;
    }  

    self->scl_write(1);
    self->sda_write(1);
    
    return UL_EOK;
}

/**
 * 软件i2c写内存函数
 *
 * @param self        软件I2C对象
 * @param devaddr     设备地址
 * @param memaddr     内存地址
 * @param pdata       要发送的数据地址
 * @param datalen     要发送的数据长度
 *
 * @return 错误代码
 */
ul_ecode ul_swi2c_mem_write(ul_swi2c_t *self, ul_uint16_t devaddr, ul_uint16_t memaddr, ul_uint8_t *pdata, ul_uint16_t datalen)
{
    ul_ecode ecode;
    
    if (self == NULL)
    {
        return UL_ENULL;
    }
    
    i2c_start(self);
    i2c_sendbyte(self, devaddr);
    /* 等待从机应答 */
    ecode = i2c_waitack(self);
    /* 从机无应答 */
    if (ecode != UL_EOK)
    {
        goto error;
    }
    i2c_sendbyte(self, memaddr);
    /* 等待从机应答 */
    ecode = i2c_waitack(self);
    /* 从机无应答 */
    if (ecode != UL_EOK)
    {
        goto error;
    }

    for(ul_uint16_t i = 0; i < datalen; i++)
    {
        i2c_sendbyte(self, pdata[i]);
        /* 等待从机应答 */
        ecode = i2c_waitack(self);
        /* 从机无应答 */
        if (ecode != UL_EOK)
        {
            goto error;
        }
    }
    
    i2c_stop(self);
    
    return UL_EOK;
    
    error:
    i2c_stop(self);
    return ecode;
}

/**
 * 软件i2c读内存函数
 *
 * @param self        软件I2C对象
 * @param devaddr     设备地址
 * @param memaddr     内存地址
 * @param pdata       要发送的数据地址
 * @param datalen     要发送的数据长度
 *
 * @return 错误代码
 */
ul_ecode ul_swi2c_mem_read(ul_swi2c_t *self, ul_uint16_t devaddr, ul_uint16_t memaddr, ul_uint8_t *pdata, ul_uint16_t datalen)
{
    ul_ecode ecode;
    
    if (self == NULL)
    {
        return UL_ENULL;
    }
    
    i2c_start(self);
    i2c_sendbyte(self, devaddr);
    /* 等待从机应答 */
    ecode = i2c_waitack(self);
    /* 从机无应答 */
    if (ecode != UL_EOK)
    {
        goto error;
    }
    i2c_sendbyte(self, memaddr);
    /* 等待从机应答 */
    ecode = i2c_waitack(self);
    /* 从机无应答 */
    if (ecode != UL_EOK)
    {
        goto error;
    }
    //i2c_stop(self);
    
    i2c_start(self);
    i2c_sendbyte(self, devaddr + 1);
    /* 等待从机应答 */
    ecode = i2c_waitack(self);
    /* 从机无应答 */
    if (ecode != UL_EOK)
    {
        goto error;
    }
    for(ul_uint16_t i = 0; i < datalen; i++)
    {
        pdata[i] = i2c_readbyte(self);
        //if (datalen == 1)
        if (i == datalen - 1)
        {
            i2c_sendnack(self);
        }
        else i2c_sendack(self);
    }
    
    i2c_stop(self);
    
    return UL_EOK;
    
    error:
    i2c_stop(self);
    return ecode;
}

/**
 * 软件i2c检查从设备是否准备好进行通信
 *
 * @param self        软件I2C对象
 * @param devaddr     设备地址
 * @param memaddr     内存地址
 *
 * @return 错误代码
 */
ul_ecode ul_swi2c_decice_check(ul_swi2c_t *self, ul_uint16_t devaddr)
{
    ul_ecode ecode;
    
    if (self == NULL)
    {
        return UL_ENULL;
    }
    
    i2c_start(self);
    i2c_sendbyte(self, devaddr);
    /* 等待从机应答 */
    ecode = i2c_waitack(self);
    /* 从机无应答 */
    if (ecode != UL_EOK)
    {
        goto error;
    }

    i2c_stop(self);
    
    return UL_EOK;
    
    error:
    i2c_stop(self);
    return ecode;
}
