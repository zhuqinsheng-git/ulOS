/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *_
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-21     zhuqinsheng   the first version
 */
#ifndef _UL_INA226_H
#define _UL_INA226_H

#include "ul_config.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*
 * ina226 structure
 */
typedef struct ul_ina226 ul_ina226_t;
struct ul_ina226
{
    ul_uint8_t addr;
    /* 移植接口 */
    
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
    ul_ecode (*i2c_write)(ul_ina226_t *self, ul_uint8_t addr, ul_uint16_t memaddr, ul_uint8_t* pdata, ul_uint16_t datalen);
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
    ul_ecode (*i2c_read)(ul_ina226_t *self, ul_uint8_t addr, ul_uint16_t memaddr, ul_uint8_t* pdata, ul_uint16_t datalen);
};

/*
 * ina226 interface
 */
ul_ecode ul_ina226_init(ul_ina226_t *self, ul_uint8_t addr, uint8_t average);
ul_ecode ul_ina226_update(ul_ina226_t *self);
ul_ecode ul_ina226_get_bus_voltage(ul_ina226_t *self, float *bus_voltage);
ul_ecode ul_ina226_get_shunt_voltage(ul_ina226_t *self, int16_t *shunt_voltage);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
