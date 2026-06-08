/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *_
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-12     zhuqinsheng   the first version
 */
#ifndef _UL_SOFTWARESPI_H
#define _UL_SOFTWARESPI_H

#include "ul_config.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 * softwarespi structure
 */
struct ul_softwarespi
{ 
    uint8_t mode;
    /* 移植接口 */
    
    /**
      * @brief 设置SCK引脚电平
      * @param pin_state 为1时实现SCK高电平，为0时实现SCK低电平
     */
    void (*sck_write)(ul_uint8_t pin_state);
    /**
      * @brief 设置MOSI引脚电平
      * @param pin_state 为1时实现MOSI高电平，为0时实现MOSI低电平
     */
    void (*mosi_write)(ul_uint8_t pin_state);
    /**
      * @brief 读取MISO引脚电平
      * @attention MISO引脚读取到高电平时必须返回1，读取到低电平时必须返回0
     */
    ul_uint8_t (*miso_read)(void);
};
typedef struct ul_softwarespi ul_swspi_t;

/*
 * softwarespi interface
 */
ul_ecode ul_swspi_init(ul_swspi_t *self, uint8_t spi_mode);
ul_uint8_t ul_swspi_rwbyte(ul_swspi_t *self, ul_uint8_t wbyte);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
