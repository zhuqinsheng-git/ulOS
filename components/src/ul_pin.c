/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-7-17     zhuqinsheng   the first version for ul_pin
 * 2025-8-13     zhuqinsheng   修改了ul_pin_t的一些属性, 主要增加了type成员
 * 2025-8-23     zhuqinsheng   修改了一些函数命名
 * 2025-9-3      zhuqinsheng   修改了初始化方法
 * 2025-11-1     zhuqinsheng   修改了框架，增加注册查找机制，进一步解耦
 */

#include "ul_pin.h"

#define PIN_STAGE_NEW_CYCLE     0
#define PIN_STAGE_OFF           1
#define PIN_STAGE_PERIOD_END    2
#define PIN_STAGE_IDLE          3

static ul_list_t pin_list_head_node = UL_LIST_HEAD_INIT(pin_list_head_node);

static ul_ecode get_ctrl_data(ul_pin_t * self)
{
    if (self->priv.ctrl.data_update == 0)
    {
        return UL_EFULL;
    }

    self->priv.ctrl.data_update = 0;
    return UL_EOK;
}

/**
 * 核心状态机
 */
static void ul_pin_task_handler(ul_pin_t * self, ul_uint32_t period)
{
    if (self == NULL)
    {
        return;
    }
    if (get_ctrl_data(self) == UL_EOK)
    {
        self->priv.stage = PIN_STAGE_NEW_CYCLE;
    }

    switch (self->priv.stage)
    {
    case PIN_STAGE_NEW_CYCLE:
    {
        if (self->priv.ctrl.ticks_set > 0)
        {
            self->set_pin(self, 1);

            if (self->priv.ctrl.ticks_clear > 0)
            {
                self->priv.ticks_count = 0;
                self->priv.stage = PIN_STAGE_OFF;
            }
            else
            {
                self->priv.stage = PIN_STAGE_IDLE;
            }
        }
        else
        {
            self->set_pin(self, 0);
            self->priv.stage = PIN_STAGE_IDLE;
        }

        break;
    }

    case PIN_STAGE_OFF:
    {
        self->priv.ticks_count += period;

        if (self->priv.ticks_count >= self->priv.ctrl.ticks_set)
        {
            self->set_pin(self, 0);
            self->priv.stage = PIN_STAGE_PERIOD_END;
        }

        break;
    }

    case PIN_STAGE_PERIOD_END:
    {
        self->priv.ticks_count += period;

        if (self->priv.ticks_count >= (self->priv.ctrl.ticks_clear + self->priv.ctrl.ticks_set))
        {
            self->priv.ticks_count -= (self->priv.ctrl.ticks_clear + self->priv.ctrl.ticks_set);

            if (self->priv.ctrl.repeat == 1)
            {
                self->set_pin(self, 0);
                self->priv.stage = PIN_STAGE_IDLE;
            }
            else
            {
                self->set_pin(self, 1);
                self->priv.ctrl.repeat = self->priv.ctrl.repeat == 0 ? 0 : self->priv.ctrl.repeat - 1;
                self->priv.stage = PIN_STAGE_OFF;
            }
        }

        break;
    }

    case PIN_STAGE_IDLE:
    {
        break;
    }

    default:
        break;
    }
}


/**
 * 初始化一个静态的pin对象，并将其节点插入到pin链表的最后
 *
 * @param self pin对象的地址
 *
 * @return 错误代码
 */
ul_ecode ul_pin_init(ul_pin_t * self, ul_uint8_t id, const char *name, func_set_pin set_pin)
{
    if (self == UL_NULL || set_pin == UL_NULL)
    {
        return UL_ERROR;
    }
    
    ul_object_init(name, &self->parent, UL_OBJECT_CLASS_PIN);
    
    /* 对象节点插入链表最后 */
    ul_list_insert_before(&pin_list_head_node, &self->priv.node);   // 节点插入到链表最后一个
    
    self->parent.flag |= UL_OBJECT_FLAG_STATIC;
    self->priv.stage = PIN_STAGE_IDLE;
    self->priv.id = id;
    self->set_pin = set_pin;
    
    return UL_EOK;
}

/**
 * 动态创建一个pin对象，并将其节点插入到pin链表的最后
 *
 * @return pin对象的地址
 */
ul_pin_t* ul_pin_create(ul_uint8_t id, const char *name, func_set_pin set_pin)
{
    ul_pin_t * self;

    self = (ul_pin_t *)ul_malloc(sizeof(struct ul_pin));

    if (self == NULL)
    {
        return UL_NULL;
    }
    
    ul_object_init(name, &self->parent, UL_OBJECT_CLASS_PIN);
    
    /* 对象节点插入链表最后 */
    ul_list_insert_before(&pin_list_head_node, &self->priv.node);   // 节点插入到链表最后一个
    
    self->parent.flag |= UL_OBJECT_FLAG_DYNAMIC;
    self->priv.stage = PIN_STAGE_IDLE;
    self->priv.id = id;
    self->set_pin = set_pin;
    
    return self;
}

/**
 * 删除一个pin动态对象，并将其节点从pin链表移除
 *
 * @return 错误代码
 */
ul_ecode ul_pin_delete(ul_pin_t * self)
{
    if (self == NULL)
    {
        return UL_ERROR;
    }
    /* 从pin链表移除 */
    ul_list_del_init(&self->priv.node);
    
    /* 动态的对象需要释放内存 */
    if (self->parent.flag & UL_OBJECT_FLAG_DYNAMIC)
    {
        ul_free(self);
    }
    
    return UL_EOK;
}

/**
 * pin定时器回调函数，需以一个固定周期调用此函数
 *
 * @param period 周期ms
 */
void ul_pin_timer_callback(uint32_t period)
{
    ul_list_t * now;
    
    /* 遍历button链表 */
    ul_list_for_each(now, &pin_list_head_node)
    {
        ul_pin_task_handler(ul_list_entry(now, struct ul_pin, priv.node), period); // 刷新设备状态
    }
}

/**
 * pin输出函数，可以使pin持续输出高电平
 *
 * @param self pin对象地址
 *
 * @return 错误代码
 */
ul_ecode ul_pin_set(ul_pin_t * self)
{
    if (self == NULL)
    {
        return UL_ERROR;
    }
    
    self->priv.ctrl.data_update  = 1;
    self->priv.ctrl.ticks_set    = 1;
    self->priv.ctrl.ticks_clear  = 0;
    self->priv.ctrl.repeat       = 0;
    
    return UL_EOK;
}

/**
 * pin输出函数，可以使pin持续输出低电平
 *
 * @param self pin对象地址
 *
 * @return 错误代码
 */
ul_ecode ul_pin_clear(ul_pin_t * self)
{
    if (self == NULL)
    {
        return UL_ERROR;
    }
    self->priv.ctrl.data_update  = 1;
    self->priv.ctrl.ticks_set    = 0;
    self->priv.ctrl.ticks_clear  = 0;
    self->priv.ctrl.repeat       = 0;
    
    return UL_EOK;
}

/**
 * pin输出脉冲，可以非阻塞使pin以设定频率输出设定脉冲
 *
 * @param self pin对象地址
 * @param ticks_set 高电平时间
 * @param ticks_clear 低电平时间
 * @param repeat 重复次数
 *
 * @return 错误代码
 */
ul_ecode ul_pin_pulse(ul_pin_t *self, ul_uint16_t ticks_set, ul_uint16_t ticks_clear, ul_uint16_t repeat)
{
    if (self == NULL)
    {
        return UL_ERROR;
    }
    self->priv.ctrl.data_update  = 1;
    self->priv.ctrl.ticks_set    = ticks_set;
    self->priv.ctrl.ticks_clear  = ticks_clear;
    self->priv.ctrl.repeat       = repeat;
    
    return UL_EOK;
}
