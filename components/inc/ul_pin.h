/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *_
 * Change Logs:
 * Date           Author       Notes
 * 2025-7-17     zhuqinsheng   the first version for ul_pin
 * 2025-8-13     zhuqinsheng   修改了ul_pin_t的一些属性, 主要增加了type成员
 * 2025-8-23     zhuqinsheng   修改了一些函数命名
 * 2025-9-3      zhuqinsheng   修改了初始化方法
 * 2025-11-1     zhuqinsheng   修改了框架，增加注册查找机制，进一步解耦
 */
#ifndef _UL_PIN_H
#define _UL_PIN_H

#include "ul_object.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*
 * pin structure
 */
typedef struct ul_pin ul_pin_t;
struct ul_pin 
{
    ul_object_t parent;

    struct
    {
        ul_list_t       node;
        
        ul_uint8_t      id;
        ul_uint8_t      stage;
        ul_uint16_t     ticks_count;
        struct 
        {
            ul_uint8_t  data_update;
            ul_uint8_t  repeat;
            ul_uint16_t ticks_set;
            ul_uint16_t ticks_clear;
        } ctrl;
    }priv;
    
    /* 移植接口 */
    /**
      * @brief 设置 PIN设备状态 （例如LED亮灭，蜂鸣器开关）
      * @param new_state PIN设备新的状态 
      * @attention 以LED为例，无论实际电路是高电平亮起还是低电平亮起，此函数都必须实现写1时亮起，写0时熄灭
     */
    void (*set_pin)(ul_pin_t* self, ul_uint32_t new_state);
};
typedef void (*func_set_pin)(ul_pin_t* self, ul_uint32_t new_state);

/*
 * pin interface
 */
ul_ecode ul_pin_init(ul_pin_t * self, ul_uint8_t id, const char *name, func_set_pin set_pin);
ul_pin_t* ul_pin_create(ul_uint8_t id, const char *name, func_set_pin set_pin);
ul_ecode ul_pin_delete(ul_pin_t *self);

void ul_pin_timer_callback(ul_uint32_t period);

ul_ecode ul_pin_set(ul_pin_t *self);
ul_ecode ul_pin_clear(ul_pin_t *self);
ul_ecode ul_pin_pulse(ul_pin_t *self, ul_uint16_t ticks_set, ul_uint16_t ticks_clear, ul_uint16_t repeat);

/**
 * 获取引脚ID
 * @param self: 引脚对象指针
 * @return 引脚ID号
 */
ul_inline ul_uint8_t ul_pin_get_id(ul_pin_t * self)
{
    return self->priv.id;
}

/**
 * 根据名称查找引脚对象
 * @param name: 引脚名称
 * @return 找到的引脚对象指针，未找到返回NULL
 */
ul_inline ul_pin_t* ul_pin_find(const char *name)
{
    return (ul_pin_t*)ul_object_find(name, UL_OBJECT_CLASS_PIN);
}


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif

