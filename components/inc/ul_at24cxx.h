/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-11     zhuqinsheng   the first version
 */
#ifndef _UL_AT24CXX_H
#define _UL_AT24CXX_H

#include "ul_config.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*
 * at24cxx structure
 */
typedef struct ul_dev_at24cxx ul_dev_at24cxx_t;
struct ul_dev_at24cxx
{
    ul_uint8_t addr;
    ul_uint8_t page_size;
    ul_uint8_t max_page;
    
    /* 移植接口 */
    /**
      * @brief at24cxx读时需要的延时函数
      *
      * @param ms 延时时间（ms）
      *
      * @return 错误代码，正常返回0，错误返回非0数即可
     */
    void (*delay_ms)(ul_int32_t ms);
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
    ul_ecode (*i2c_write)(ul_dev_at24cxx_t *self, ul_uint16_t memaddr, ul_uint8_t* pdata, ul_uint16_t datalen);
    /**
      * @brief at24cxx的i2c读函数
      *
      * @param self at24cxx对象（自己）
      * @param memaddr     内存地址
      * @param pdata       读取到的数据首地址
      * @param datalen     要读取的数据长度
      *
      * @return 错误代码，正常返回0，错误返回非0数即可
     */
    ul_ecode (*i2c_read)(ul_dev_at24cxx_t *self, ul_uint16_t memaddr, ul_uint8_t* pdata, ul_uint16_t datalen);
};


/*
 * at24cxx interface
 */
ul_ecode ul_at24cxx_init(ul_dev_at24cxx_t *self, ul_uint8_t addr,ul_uint8_t page_size, ul_uint16_t max_page);
ul_ecode ul_at24cxx_write(ul_dev_at24cxx_t *self, ul_uint16_t memaddr, ul_uint8_t* pdata, ul_uint16_t datalen);
ul_ecode ul_at24cxx_read(ul_dev_at24cxx_t *self, ul_uint16_t memaddr, ul_uint8_t* pdata, ul_uint16_t datalen);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
