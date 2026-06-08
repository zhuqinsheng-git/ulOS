/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *_
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-11     zhuqinsheng   the first version
 * 2025-8-22     zhuqinsheng   修复了一些通信时序问题
 */
#ifndef _UL_SOFTWAREIIC_H
#define _UL_SOFTWAREIIC_H

#include "ul_config.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 * softwarei2c structure
 */
struct ul_softwarei2c
{ 
    /* 移植接口 */
    
    /**
      * @brief 设置延时时间
      * @attention 无参数，所以需要延时多久，直接在接口处实现
     */
    void (*delay_xxus)(void);
    /**
      * @brief 设置SCL引脚电平
      * @param pin_state 为1时实现SCL高电平，为0时实现SCL低电平
     */
    void (*scl_write)(ul_uint8_t pin_state);
    /**
      * @brief 设置SDA引脚电平
      * @param pin_state 为1时实现SCL高电平，为0时实现SCL低电平
     */
    void (*sda_write)(ul_uint8_t pin_state);
    /**
      * @brief 读取SDA引脚电平
      * @attention SDA引脚读取到高电平时必须返回1，读取到低电平时必须返回0
     */
    ul_uint8_t (*sda_read)(void);
};
typedef struct ul_softwarei2c ul_swi2c_t;

/*
 * softwarei2c interface
 */
ul_ecode ul_swi2c_init(ul_swi2c_t *self);
ul_ecode ul_swi2c_mem_write(ul_swi2c_t *self, ul_uint16_t devaddr, ul_uint16_t memaddr, ul_uint8_t *pdata, ul_uint16_t datalen);
ul_ecode ul_swi2c_mem_read(ul_swi2c_t *self, ul_uint16_t devaddr, ul_uint16_t memaddr, ul_uint8_t *pdata, ul_uint16_t datalen);
ul_ecode ul_swi2c_decice_check(ul_swi2c_t *self, ul_uint16_t devaddr);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
