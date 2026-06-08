/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-13     zhuqinsheng   the first version
 * 2025-11-25    zhuqinsheng   适配ulOS
 */
#ifndef _UL_SWTIMER_H
#define _UL_SWTIMER_H

#include "ul_config.h"
#include "ul_object.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define UL_SWTIMER_TYPE_ONE_SHOT          0x01             /**< one shot timer */
#define UL_SWTIMER_TYPE_PERIODIC          0x02             /**< periodic timer */
#define UL_SWTIMER_TYPE_STATIC            0x04             /**< periodic timer */
#define UL_SWTIMER_TYPE_DYNAMIC           0x08             /**< periodic timer */

#define UL_SWTIMER_TYPE_URGENT            0x10             /**< periodic timer */

/*
 * swtimer structure
 */
struct ul_software_timer
{
    uint8_t ready;                // 调度标志，1：调度，0：挂起
    int16_t count;          // 时间片计数值
    uint16_t reload;          // 时间片重载值
    void (*entry)(void *parameter);  //函数指针变量，用来保存业务功能模块函数地址
    void *parameter;

    uint32_t type;   // [0]静态创建  [1]动态创建
    ul_list_t node;
};
typedef struct ul_software_timer ul_swtimer_t;
typedef void (*swtimer_entry_t)(void *parameter);

/*
 * swtimer interface
 */
void ul_swtimer_handler(void);
void ul_swtimer_tick_callback(ul_uint8_t period);

ul_ecode ul_swtimer_init(ul_swtimer_t *self, ul_uint16_t reload, void *parameter, swtimer_entry_t func, uint8_t mode);
ul_swtimer_t* ul_swtimer_create(ul_uint16_t reload, void *parameter, swtimer_entry_t func, uint8_t mode);
ul_ecode ul_swtimer_delete(ul_swtimer_t * self);

ul_ecode ul_swtimer_start(ul_swtimer_t * self);
ul_ecode ul_swtimer_stop(ul_swtimer_t * self);

ul_ecode ul_swtimer_system_start(void);


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
