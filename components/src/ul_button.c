/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-7-21     zhuqinsheng   the first version
 * 2025-8-13     zhuqinsheng   修改了ul_button_t的一些属性, 主要增加了type成员
 * 2025-12-9     zhuqinsheng   优化了初始化机制,新增长按循环触发功能
 */
#include "ul_button.h"
#include "ul_object.h"

struct ul_button
{
    ul_uint32_t             id;
    ul_uint8_t              stage;
    ul_uint32_t             long_ticks_count;
    ul_uint32_t             double_ticks_count;

    struct
    {
        ul_uint32_t         dp_th; /* 双击时间阈值ms */
        ul_uint32_t         lp_th; /* 长按时间阈值ms */
    } config;

    ul_button_event_cb_func event_callback;
    button_event_e          event;
    ul_uint8_t              type;
    ul_list_t               node;

    /* 移植接口 */
    /**
      * @brief 读取button状态
      * @attention 用户实现此接口时其返回值必须为按下时为1,松开时为0
      * @return 0 按钮未被按下
      * @return 1 按钮已被按下
    */
    ul_button_read_fuc read_pin;
};

/* 按键类型定义 */
#define UL_BUTTON_TYPE_STATIC      0x01    /* 静态创建的按键 */
#define UL_BUTTON_TYPE_DYNAMIC     0x02    /* 动态创建的按键 */

#define BUTTON_STAGE_NORMAL         0
#define BUTTON_STAGE_SHAKE          1
#define BUTTON_STAGE_LONGPRESS      2
#define BUTTON_STAGE_DOUBLEPRESS    3
#define BUTTON_STAGE_IDLE           4
#define BUTTON_STAGE_REPEAT_PRESS   5

static ul_list_t button_list_head_node = UL_LIST_HEAD_INIT(button_list_head_node);

static void ul_button_task_handler(ul_button_t * self, ul_uint32_t period)
{
    if (self == NULL)
    {
        return;
    }

    ul_uint32_t pin = self->read_pin();

    switch (self->stage)
    {
    case BUTTON_STAGE_NORMAL:
    {
        if (pin == 1)
        {
            self->stage = BUTTON_STAGE_SHAKE;
        }
    }
    break;

    case BUTTON_STAGE_SHAKE:
    {
        if (pin == 1)
        {
            self->stage = BUTTON_STAGE_LONGPRESS;
            self->long_ticks_count = 0;  // 重置长按计数
        }
        else
        {
            self->stage = BUTTON_STAGE_NORMAL;
        }
    }
    break;

    case BUTTON_STAGE_LONGPRESS:
    {
        if (pin == 0)
        {
            self->stage = BUTTON_STAGE_DOUBLEPRESS;
        }
        else
        {
            self->long_ticks_count += period;

            if (self->long_ticks_count >= self->config.lp_th)
            {
                self->event_callback(UL_BUTTON_EVENT_LONGPRESS);
                self->stage = BUTTON_STAGE_REPEAT_PRESS;  // 进入连续长按状态
                self->long_ticks_count = 0;  // 重置计数器
            }
        }
    }
    break;

    case BUTTON_STAGE_REPEAT_PRESS:  // 新增的连续长按状态处理
    {
        if (pin == 0)
        {
            self->stage = BUTTON_STAGE_NORMAL;
        }
        else
        {
            self->long_ticks_count += period;

            if (self->long_ticks_count >= self->config.lp_th)
            {
                self->event_callback(UL_BUTTON_EVENT_LONGPRESS);
                self->long_ticks_count = 0;  // 重置计数器，准备下一次连续长按
            }
        }
    }
    break;

    case BUTTON_STAGE_DOUBLEPRESS:
    {
        self->double_ticks_count += period;

        if (pin == 1)
        {
            self->event_callback(UL_BUTTON_EVENT_DOUBLE_CLICK);
            self->stage = BUTTON_STAGE_IDLE;
        }
        else if (self->double_ticks_count > self->config.dp_th)
        {
            self->event_callback(UL_BUTTON_EVENT_PRESSED);
            self->stage = BUTTON_STAGE_IDLE;
        }
    }
    break;

    case BUTTON_STAGE_IDLE:
    {
        if (pin == 0)
        {
            self->double_ticks_count = 0;
            self->long_ticks_count = 0;
            self->stage = BUTTON_STAGE_NORMAL;
        }
    }
    break;
    }
}
/**
 * 注册button事件回调函数
 *
 * @param self button对象的地址
 * @param ul_button_event_cb_func 要注册的事件回调函数地址
 */
void ul_button_register_callback(ul_button_t * self, ul_button_event_cb_func func)
{
    if (func == NULL)
    {
        return;
    }

    self->event_callback = func;
}

/**
 * button对象默认的事件回调函数
 *
 * @param self button对象的地址
 * @param event 事件代码
 */
ul_weak void ul_button_defalut_event_callback(button_event_e event)
{

}

/**
 * 初始化一个静态的button对象，并将其节点插入到button链表的最后
 *
 * @param self button对象的地址
 *
 * @return 错误代码
 */
ul_ecode ul_button_init(ul_button_t *self,
                               ul_button_read_fuc read_pin,
                               ul_button_event_cb_func cb_func)
{
    if (self == UL_NULL || read_pin == UL_NULL)
    {
        return UL_ERROR;
    }

    self->read_pin = read_pin;
    self->long_ticks_count      = 0;
    self->double_ticks_count    = 0;
    self->config.lp_th = UL_BUTTON_CONFIG_LP_TIME;
    self->config.dp_th = UL_BUTTON_CONFIG_DP_TIME;

    if (cb_func == UL_NULL)
    {
        self->event_callback    = ul_button_defalut_event_callback;
    }
    else
    {
        self->event_callback    = cb_func;
    }

    /* 对象节点插入链表最后 */
    ul_list_insert_before(&button_list_head_node, &self->node);   // 节点插入到链表最后一个

    self->type                  |= UL_BUTTON_TYPE_STATIC;
    self->stage = BUTTON_STAGE_NORMAL;
    return UL_EOK;
}

/**
 * 动态创建一个button对象，并将其节点插入到button链表的最后
 *
 * @return button对象的地址
 */
ul_button_t *ul_button_create(ul_button_read_fuc read_pin,
                                     ul_button_event_cb_func cb_func)
{
    if (read_pin == UL_NULL)
    {
        return UL_NULL;
    }

    ul_button_t *self;

    self = (ul_button_t *)ul_malloc(sizeof(struct ul_button));

    if (self == NULL)
    {
        return UL_NULL;
    }

    self->read_pin = read_pin;
    self->long_ticks_count      = 0;
    self->double_ticks_count    = 0;
    self->config.lp_th = UL_BUTTON_CONFIG_LP_TIME;
    self->config.dp_th = UL_BUTTON_CONFIG_DP_TIME;
    self->stage                = BUTTON_STAGE_NORMAL;

    if (cb_func == UL_NULL)
    {
        self->event_callback        = ul_button_defalut_event_callback;
    }
    else
    {
        self->event_callback        = cb_func;
    }

    self->type                |= UL_BUTTON_TYPE_DYNAMIC;
    /* 对象节点插入链表最后 */
    ul_list_insert_before(&button_list_head_node, &self->node);


    return self;
}

/**
 * 删除一个动态的button对象，并将其节点从button链表移除
 *
 * @return 错误代码
 */
ul_ecode ul_button_delete(ul_button_t * self)
{
    if (self == NULL)
    {
        return UL_ERROR;
    }

    /* 从button链表移除 */
    ul_list_del_init(&self->node);

    /* 动态的对象需要释放内存 */
    if (self->type & UL_BUTTON_TYPE_DYNAMIC)
    {
        ul_free(self);
    }

    return UL_EOK;
}

/**
 * 设置button长按和双击的时间阈值
 *
 * @param self button对象的地址
 * @param longpress_time 长按时间阈值
 * @param doublepress_time 双击时间阈值
 */
void ul_button_presstime_change(ul_button_t *self, ul_uint32_t lp_time, ul_uint32_t dp_time)
{
    self->config.lp_th = lp_time;
    self->config.dp_th = dp_time;
}

/**
 * button定时器回调函数，需以一个固定周期调用此函数
 *
 * @param period 周期ms
 */
void ul_button_timer_callback(ul_uint32_t period)
{
    ul_list_t *now;

    /* 遍历button链表 */
    ul_list_for_each(now, &button_list_head_node)
    {
        ul_button_task_handler(ul_list_entry(now, struct ul_button, node), period);
    }
}
