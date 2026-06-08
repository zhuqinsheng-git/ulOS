/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *_
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-12     zhuqinsheng   the first version
 * 2025-9-07     zhuqinsheng   将擦除和写入分开
 */
#ifndef _UL_W25QXX_H
#define _UL_W25QXX_H

#include "ul_config.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*
 * w25qxx structure
 */
typedef struct ul_dev_w25qxx ul_dev_w25qxx_t;
struct ul_dev_w25qxx
{
    ul_uint8_t xx;
    
    /* 移植接口 */
    
    /**
      * @brief 设置CS引脚电平
      * @param pin_state 为1时实现CS高电平，为0时实现CS低电平
      * @attention CS引脚应配置为推挽输出，初始电平为高电平
     */
    void (*cs_write)(ul_uint8_t pin_state);
    /**
      * @brief at24cxx的i2c写函数
      *
      * @param self at24cxx对象（自己）
      * @param memaddr     内存地址
      * @param pdata       要写入的数据首地址
      * @param datalen     要写入的数据长度
      *
      * @return 错误代码，正常返回0，错误返回非0数即可
     */
    ul_uint8_t (*spi_rwbyte)(ul_uint8_t wbyte);
};

/*
 * w25qxx interface
 */
ul_ecode ul_w25qxx_init(ul_dev_w25qxx_t *self, ul_uint8_t xx);
uint16_t ul_w25qxx_checkid(ul_dev_w25qxx_t *self);
void ul_w25qxx_erase(ul_dev_w25qxx_t *self, ul_uint32_t erase_addr, ul_uint32_t datalen);
void ul_w25qxx_write(ul_dev_w25qxx_t *self, ul_uint32_t memaddr, ul_uint8_t *pdata, ul_uint16_t datalen);
void ul_w25qxx_read(ul_dev_w25qxx_t *self, ul_uint32_t memaddr, ul_uint8_t *pdata, ul_uint16_t datalen);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
