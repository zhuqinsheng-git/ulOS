/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-11     zhuqinsheng   the first version
 */
#include "ul_at24cxx.h"

/**
 * AT24CXX初始化函数
 *
 * @param self        AT24CXX对象
 * @param addr        AT24CXX设备地址
 * @param pdata       页大小（字节）
 * @param datalen     页数量
 *
 * @return 错误代码
 */
ul_ecode ul_at24cxx_init(ul_dev_at24cxx_t *self, ul_uint8_t addr,ul_uint8_t page_size, ul_uint16_t max_page)
{
    if (self == NULL)
    {
        return UL_ENULL;
    }
    
    self->addr      = addr;
    self->page_size = page_size;
    self->max_page  = max_page;
    
    return UL_EOK;
}

/**
 * AT24CXX写函数
 *
 * @param self        AT24CXX对象
 * @param memaddr     内存地址
 * @param pdata       要写入的数据首地址
 * @param datalen     要写入的数据长度
 *
 * @return 错误代码
 */
ul_ecode ul_at24cxx_write(ul_dev_at24cxx_t *self, ul_uint16_t memaddr, ul_uint8_t* pdata, ul_uint16_t datalen)
{
    if (NULL == self) 
    {
        return UL_ENULL;
    }
    if (0 == datalen) 
    { 
        return UL_ERROR; 
    }
    if (NULL == self->i2c_write)
    {
        return UL_ENULL;
    }
    if (NULL == self->i2c_read)
    {
        return UL_ENULL;
    }
    int res = UL_EOK;
    
    int selectPage_idx  = memaddr % self->page_size;
    int selectPage_rest = self->page_size - selectPage_idx;
    
    if (datalen <= selectPage_rest) 
    {
        res = self->i2c_write(self,
                         memaddr,
                         pdata,
                         datalen);
        
        if (UL_EOK != res) 
        {
            return UL_ERROR;
        }        
    } 
    else 
    {
    
        /*! 1 write selectPage rest*/
        res = self->i2c_write(self,
                         memaddr,
                         pdata,
                         selectPage_rest);
        
        if (UL_EOK != res) 
        {
            return UL_ERROR;
        }
        
        memaddr += selectPage_rest;
        datalen -= selectPage_rest;
        pdata   += selectPage_rest;
        
        self->delay_ms(5);
        
        /*! 2 write nextPage full */
        int fullPage = datalen/self->page_size;
        for (int iPage = 0; iPage < fullPage; ++iPage) 
        {
            res = self->i2c_write(self,
                             memaddr,
                             pdata,
                             self->page_size);
            if (UL_EOK != res) 
            {
                return UL_ERROR;
            }
            
            self->delay_ms(5);
            
            memaddr += self->page_size;
            datalen -= self->page_size;
            pdata   += self->page_size;
        }
        
        /*! 3 write rest */
        if (0 != datalen) 
        {
            res = self->i2c_write(self,
                             memaddr,
                             pdata,
                             datalen);
        
            if (UL_EOK != res) 
            {
                return UL_ERROR;
            }
        }
    }
    
    return UL_EOK;
}
 
/**
 * AT24CXX读函数
 *
 * @param self        AT24CXX对象
 * @param memaddr     内存地址
 * @param pdata       接收的数据地址
 * @param datalen     接收的数据长度
 *
 * @return 错误代码
 */
ul_ecode ul_at24cxx_read(ul_dev_at24cxx_t *self, ul_uint16_t memaddr, ul_uint8_t* pdata, ul_uint16_t datalen)
{
    if (NULL == self) 
    {
        return UL_ENULL;
    }
    if (0 == datalen) 
    { 
        return UL_ERROR; 
    }
    if (NULL == self->i2c_write)
    {
        return UL_ENULL;
    }
    if (NULL == self->i2c_read)
    {
        return UL_ENULL;
    }
    int res = self->i2c_read(self, 
                         memaddr,
                         pdata,
                         datalen);
    
    if (UL_EOK != res) 
    {
        return UL_ERROR;
    }
    
    return UL_EOK;
}

//AT24C02测试代码
#if 0
ul_swi2c_t swi2c_at24cxx;

ul_ecode write(ul_dev_at24cxx_t *self, ul_uint16_t memaddr, ul_uint8_t* pdata, ul_uint16_t datalen)
{
    return ul_swi2c_mem_write(&swi2c_at24cxx, self->addr,memaddr,pdata,datalen);
}

ul_ecode read(ul_dev_at24cxx_t *self, ul_uint16_t memaddr, ul_uint8_t* pdata, ul_uint16_t datalen)
{
    return ul_swi2c_mem_read(&swi2c_at24cxx, self->addr,memaddr,pdata,datalen);
}

#define AT24CXX_TEST_NUM    12
    uint8_t c02datawrite[AT24CXX_TEST_NUM] = {1};
    uint8_t c02dataread[AT24CXX_TEST_NUM] = {0};
    ul_dev_at24cxx_t at24c02;
    swi2c_at24cxx.delay_xxus = delay_xxus;
    swi2c_at24cxx.scl_write = scl_write;
    swi2c_at24cxx.sda_read = sda_read;
    swi2c_at24cxx.sda_write = sda_write;
    ul_swi2c_init(&swi2c_at24cxx);
    at24c02.i2c_write = write;
    at24c02.i2c_read = read;
    ul_at24cxx_init(&at24c02,0xa0,8,256);
    for(int i = 0; i < AT24CXX_TEST_NUM; i++)
    {
        c02datawrite[i] = i+1;
    }
    
    ul_at24cxx_write(&at24c02,0, c02datawrite, AT24CXX_TEST_NUM);
    ul_delay_ms(10);
    ul_at24cxx_read(&at24c02,0, c02dataread, AT24CXX_TEST_NUM);
    for(int i = 0; i < AT24CXX_TEST_NUM; i++)
    {
        if (c02datawrite[i] != c02dataread[i])
        {
            while(1);
        }
    }
#endif
