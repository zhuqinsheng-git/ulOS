/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-13     zhuqinsheng   the first version
 */
#include "ul_swtimer.h"

static ul_list_t swtimer_list_head_node = UL_LIST_HEAD_INIT(swtimer_list_head_node);
static ul_uint8_t ul_swtimer_system_start_flag = 0;

/**
 * 软件定时器处理函数，直接放在主函数的while循环内即可
 */
void ul_swtimer_handler(void)
{
    if (ul_swtimer_system_start_flag != 1)    // 使用了ul_scheduler_start()之后才能开始调度
    {
        return;
    }
    
    ul_list_t * now;
    ul_list_t * next;
    ul_swtimer_t *now_swtimer;
    
    /* 遍历button链表 */
    ul_list_for_each_safe(now, next, &swtimer_list_head_node)
    {
        now_swtimer = ul_list_entry(now, ul_swtimer_t, node);

        if (now_swtimer->ready == 1)            //判断时间片标志
        {
            if (now_swtimer->type & UL_SWTIMER_TYPE_ONE_SHOT)
            {
                ul_list_del_init(&now_swtimer->node); // 从链表移除，也就不会继续调度
            }
            if(now_swtimer->entry == NULL)
            {
               return;
            }
            now_swtimer->entry(now_swtimer->parameter);       // 执行调度业务功能模块
            now_swtimer->ready = 0;             //标志清零
        }
    }
}

/**
 * 软件定时器tick刷新函数，需以一个固定周期调用此函数，建议放在硬件定时器中断内，且周期不大于5ms
 *
 * @param period 实际调用的周期ms
 */
void ul_swtimer_tick_callback(ul_uint8_t period)
{
    if (ul_swtimer_system_start_flag != 1)    // 使用了ul_scheduler_start()之后才能开始调度
    {
        return;
    }
    
    ul_list_t * now;
    ul_list_t * next;
    ul_swtimer_t *now_swtimer;

    /* 遍历button链表 */
    ul_list_for_each_safe(now, next, &swtimer_list_head_node)
    {
        now_swtimer = ul_list_entry(now, ul_swtimer_t, node);
        if (now_swtimer->count > 0)
        {
            now_swtimer->count -= period;
            if(now_swtimer->count <= 0)
            {
                now_swtimer->ready = 1;
                now_swtimer->count = now_swtimer->reload;
                
                if (now_swtimer->type & UL_SWTIMER_TYPE_URGENT)
                {
                    if (now_swtimer->type & UL_SWTIMER_TYPE_ONE_SHOT)
                    {
                        ul_list_del_init(&now_swtimer->node); // 从链表移除，也就不会继续调度
                    }
                    if(now_swtimer->entry == NULL)
                    {
                       return;
                    }
                    now_swtimer->entry(now_swtimer->parameter);       // 执行调度业务功能模块
                    now_swtimer->ready = 0;             //标志清零
                }
            }
        }
    }
}

/**
 * 初始化一个静态的软件定时器对象，并将其节点插入到软件定时器链表的最后
 *
 * @param self      软件定时器对象的地址
 * @param reload    软件定时器调用周期
 * @param parameter 软件定时器周期到达时传递给入口函数的参数
 * @param ready     软件定时器是否在开启时不等待周期结束直接执行（并不是立即执行），[0]否 [1]是
 * @param func      软件定时器入口函数地址
 *
 * @return 错误代码
 */
ul_ecode ul_swtimer_init(ul_swtimer_t *self, ul_uint16_t reload, void *parameter, swtimer_entry_t func, uint8_t mode)
{
    if (self == NULL)
    {
        return UL_ERROR;
    }
    if (func == NULL)
    {
        return UL_ERROR;
    }
    
    self->count     = reload;
    self->reload    = reload;
    self->ready     = 0;
    self->entry     = func;
    self->parameter = parameter;
    self->type     |= mode;
    
    self->type     |= UL_SWTIMER_TYPE_STATIC;
    /* 对象节点插入链表最后 */
    ul_list_insert_before(&swtimer_list_head_node, &self->node);   // 节点插入到链表最后一个

    return UL_EOK;
}

/**
 * 创建一个动态的软件定时器对象，并将其节点插入到软件定时器链表的最后
 *
 * @param reload    软件定时器调用周期
 * @param parameter 软件定时器周期到达时传递给入口函数的参数
 * @param ready     软件定时器是否在开启时不等待周期结束直接执行（并不是立即执行），[0]否 [1]是
 * @param func      软件定时器入口函数地址
 *
 * @return self     软件定时器对象的地址
 */
ul_swtimer_t* ul_swtimer_create(ul_uint16_t reload, void *parameter, swtimer_entry_t func, uint8_t mode)
{
    ul_swtimer_t *timer_object;
        
    timer_object = (ul_swtimer_t *)ul_malloc(sizeof(ul_swtimer_t));

    if (timer_object == NULL)
    {
        return UL_NULL;
    }
    
    timer_object->count     = reload;
    timer_object->reload    = reload;
    timer_object->ready     = 0;
    timer_object->entry     = func;
    timer_object->parameter = parameter;
    timer_object->type     |= mode;
    timer_object->type     |= UL_SWTIMER_TYPE_DYNAMIC;
    /* 对象节点插入链表最后 */
    ul_list_insert_before(&swtimer_list_head_node, &timer_object->node);

    
    return timer_object;
}

/**
 * 删除一个软件定时器对象，并将其节点从软件定时器链表移除
 *
 * @return 错误代码
 */
ul_ecode ul_swtimer_delete(ul_swtimer_t * self)
{
    if (self == NULL)
    {
        return UL_ERROR;
    }

    /* 从button链表移除 */
    ul_list_del_init(&self->node);
    
    if (self->type & UL_SWTIMER_TYPE_DYNAMIC)    // 动态创建的还要释放内存
    {
        ul_free(self);
    }

    return UL_EOK;
}

/**
 * 开启软件定时器
 *
 * @return 错误代码
 */
ul_ecode ul_swtimer_start(ul_swtimer_t * self)
{
    if (ul_list_is_empty(&self->node))    // 如果软件定时器已经开启了，不能重复开启
    {
        /* 对象节点插入链表最后 */
        ul_list_insert_before(&swtimer_list_head_node, &self->node);
        return UL_EOK;
    }
    return UL_ERROR;
}

/**
 * 关闭软件定时器
 *
 * @return 错误代码
 */
ul_ecode ul_swtimer_stop(ul_swtimer_t * self)
{
    if (ul_list_is_empty(&self->node))    // 如果软件定时器已经开启了，不能重复开启
    {
        return UL_ERROR;
    }
    ul_list_del_init(&self->node);
    return UL_EOK;
}

/**
 * 开启软件定时器
 *
 * @return 错误代码
 */
ul_ecode ul_swtimer_system_start(void)
{
    if (ul_swtimer_system_start_flag == 1)    // 如果软件定时器已经开启了，不能重复开启
    {
        return UL_ERROR;
    }
    
    ul_swtimer_system_start_flag = 1;
    
    return UL_EOK;
}
