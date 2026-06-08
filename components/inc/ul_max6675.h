/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *_
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-21     zhuqinsheng   the first version
 */
#ifndef _UL_MAX6675_H
#define _UL_MAX6675_H

#include "ul_config.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*
 * max6675 structure
 */
typedef struct ul_max6675 ul_max6675_t;
struct ul_max6675
{
    ul_fp32_t temp;
    /* 移植接口 */
    /**
      * @brief 设置CS引脚电平
      * @param pin_state 为1时实现CS高电平，为0时实现CS低电平
      * @attention CS引脚应配置为推挽输出，初始电平为高电平
     */
    void (*cs_write)(ul_uint8_t pin_state);
    /**
      * @brief spi读取半字(2字节)
      *
      * @return 读到的半字数据
     */
    ul_uint16_t (*spi_rhalfword)(void);
};

/*
 * max6675 interface
 */
ul_ecode ul_max6675_init(ul_max6675_t *self);
ul_ecode ul_max6675_update(ul_max6675_t *self);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
