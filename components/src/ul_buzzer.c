/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-7-17     zhuqinsheng   the first version for ul_buzzer
 * 2025-8-13     zhuqinsheng   修改了ul_buzzer_t的一些属性, 主要增加了type成员
 * 2025-8-23     zhuqinsheng   修改了一些函数命名
 * 2025-9-3      zhuqinsheng   修改了初始化方法
 */

#include "ul_buzzer.h"
#include "ul_object.h"

#define BUZZER_STAGE_NEW_CYCLE     0
#define BUZZER_STAGE_OFF           1
#define BUZZER_STAGE_PERIOD_END    2
#define BUZZER_STAGE_IDLE          3

static ul_list_t buzzer_list_head_node;
static ul_uint8_t ul_list_head_node_created_flag = 0;

static ul_ecode get_ctrl_data(ul_buzzer_t * self)
{
    if (self->ctrl.data_update == 0)
    {
        return UL_EFULL;
    }

    self->ctrl.data_update = 0;
    return UL_EOK;
}

static void ul_buzzer_task_handler(ul_buzzer_t * self, ul_uint32_t period)
{
    if (self == NULL)
    {
        return;
    }
    if (get_ctrl_data(self) == UL_EOK)
    {
        self->stage = BUZZER_STAGE_NEW_CYCLE;
    }

    switch (self->stage)
    {
    case BUZZER_STAGE_NEW_CYCLE:
    {
        if (self->ctrl.ticks_set > 0)
        {
            self->set_freq(self, self->freq);

            if (self->ctrl.ticks_clear > 0)
            {
                self->ticks_count = 0;
                self->stage = BUZZER_STAGE_OFF;
            }
            else
            {
                self->stage = BUZZER_STAGE_IDLE;
            }
        }
        else
        {
            self->set_freq(self, 0);
            self->stage = BUZZER_STAGE_IDLE;
        }

        break;
    }

    case BUZZER_STAGE_OFF:
    {
        self->ticks_count += period;

        if (self->ticks_count >= self->ctrl.ticks_set)
        {
            self->set_freq(self, 0);
            self->stage = BUZZER_STAGE_PERIOD_END;
        }

        break;
    }

    case BUZZER_STAGE_PERIOD_END:
    {
        self->ticks_count += period;

        if (self->ticks_count >= (self->ctrl.ticks_clear + self->ctrl.ticks_set))
        {
            self->ticks_count -= (self->ctrl.ticks_clear + self->ctrl.ticks_set);

            if (self->ctrl.repeat == 1)
            {
                self->set_freq(self, 0);
                self->stage = BUZZER_STAGE_IDLE;
            }
            else
            {
                self->set_freq(self, self->freq);
                self->ctrl.repeat = self->ctrl.repeat == 0 ? 0 : self->ctrl.repeat - 1;
                self->stage = BUZZER_STAGE_OFF;
            }
        }

        break;
    }

    case BUZZER_STAGE_IDLE:
    {
        break;
    }

    default:
        break;
    }
}

static void ul_list_head_node_create(ul_list_t *head)
{
    /* 如果没有buzzer设备的头节点，创建一个头节点 */
    if (ul_list_head_node_created_flag == 0)
    {
        ul_list_init(head);

        if (ul_list_is_empty(head))
        {
            ul_list_head_node_created_flag = 1;
        }
    }
}


/**
 * 初始化一个静态的buzzer对象，并将其节点插入到buzzer链表的最后
 *
 * @param self buzzer对象的地址
 *
 * @return 错误代码
 */
ul_ecode ul_buzzer_init(ul_buzzer_t * self, uint8_t id, func_set_freq set_freq)
{
    if (self == UL_NULL || set_freq == UL_NULL)
    {
        return UL_ERROR;
    }
    /* 第一次创建头节点 */
    ul_list_head_node_create(&buzzer_list_head_node);
    /* 对象节点插入链表最后 */
    ul_list_insert_before(&buzzer_list_head_node, &self->node);   // 节点插入到链表最后一个
    
    self->type |= UL_BUZZER_TYPE_STATIC;
    self->stage = BUZZER_STAGE_IDLE;
    self->id = id;
    self->set_freq = set_freq;
    
    return UL_EOK;
}

/**
 * 动态创建一个buzzer对象，并将其节点插入到buzzer链表的最后
 *
 * @return buzzer对象的地址
 */
ul_buzzer_t* ul_buzzer_create(uint8_t id, func_set_freq set_freq)
{
    ul_buzzer_t * buzzer_object;
    /* 第一次创建头节点 */
    ul_list_head_node_create(&buzzer_list_head_node);

    buzzer_object = (ul_buzzer_t *)ul_malloc(sizeof(struct ul_buzzer));

    if (buzzer_object == NULL)
    {
        return UL_NULL;
    }
    /* 对象节点插入链表最后 */
    ul_list_insert_before(&buzzer_list_head_node, &buzzer_object->node);   // 节点插入到链表最后一个
    
    buzzer_object->type |= UL_BUZZER_TYPE_DYNAMIC;
    buzzer_object->stage = BUZZER_STAGE_IDLE;
    buzzer_object->id = id;
    buzzer_object->set_freq = set_freq;
    
    return buzzer_object;
}

/**
 * 删除一个buzzer动态对象，并将其节点从buzzer链表移除
 *
 * @return 错误代码
 */
ul_ecode ul_buzzer_delete(ul_buzzer_t * self)
{
    if (self == NULL)
    {
        return UL_ERROR;
    }
    /* 从buzzer链表移除 */
    ul_list_del_init(&self->node);
    
    /* 动态的对象需要释放内存 */
    if (self->type & UL_BUZZER_TYPE_DYNAMIC)
    {
        ul_free(self);
    }
    
    return UL_EOK;
}

/**
 * buzzer定时器回调函数，需以一个固定周期调用此函数
 *
 * @param period 周期ms
 */
void ul_buzzer_timer_callback(uint32_t period)
{
    ul_list_t * now;
    
    /* 头节点未创建 */
    if (ul_list_head_node_created_flag == 0)
    {
        return;
    }
    /* 遍历button链表 */
    ul_list_for_each(now, &buzzer_list_head_node)
    {
        ul_buzzer_task_handler(ul_list_entry(now, struct ul_buzzer, node), period); // 刷新设备状态
    }
}

/**
 * buzzer输出函数，可以使buzzer持续输出高电平
 *
 * @param self buzzer对象地址
 *
 * @return 错误代码
 */
ul_ecode ul_buzzer_set(ul_buzzer_t * self, uint16_t freq)
{
    if (self == NULL)
    {
        return UL_ERROR;
    }
    
    self->ctrl.data_update  = 1;
    self->ctrl.ticks_set    = 1;
    self->ctrl.ticks_clear  = 0;
    self->ctrl.repeat       = 0;
    self->freq = freq;
    return UL_EOK;
}

/**
 * buzzer输出函数，可以使buzzer持续输出低电平
 *
 * @param self buzzer对象地址
 *
 * @return 错误代码
 */
ul_ecode ul_buzzer_clear(ul_buzzer_t * self)
{
    if (self == NULL)
    {
        return UL_ERROR;
    }
    self->ctrl.data_update  = 1;
    self->ctrl.ticks_set    = 0;
    self->ctrl.ticks_clear  = 0;
    self->ctrl.repeat       = 0;
    self->freq = 0;
    return UL_EOK;
}

/**
 * buzzer输出脉冲，可以非阻塞使buzzer以设定频率输出设定脉冲
 *
 * @param self buzzer对象地址
 * @param ticks_set 高电平时间
 * @param ticks_clear 低电平时间
 * @param repeat 重复次数
 *
 * @return 错误代码
 */
ul_ecode ul_buzzer_pulse(ul_buzzer_t * self, uint16_t freq, ul_uint32_t ticks_set, ul_uint32_t ticks_clear, ul_uint32_t repeat)
{
    if (self == NULL)
    {
        return UL_ERROR;
    }
    self->ctrl.data_update  = 1;
    self->ctrl.ticks_set    = ticks_set;
    self->ctrl.ticks_clear  = ticks_clear;
    self->ctrl.repeat       = repeat;
    self->freq = freq;
    return UL_EOK;
}
