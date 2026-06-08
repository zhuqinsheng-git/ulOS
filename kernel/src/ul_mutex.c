/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-4     zhuqinsheng   the first version
 */
#include "ul_mutex.h"
#include "ul_thread.h"

extern void _thread_insert_ready_list(struct ul_thread *thread);
extern void _thread_remove_ready_list(struct ul_thread *thread);
extern ul_base_t ul_hw_interrupt_disable(void);
extern void ul_hw_interrupt_enable(ul_base_t level);

// 初始化互斥锁
ul_ecode ul_mutex_init(ul_mutex_t *mutex, const char *name)
{
    if (mutex == UL_NULL)
    {
        return UL_ERROR;
    }
    
    ul_object_init(name, &mutex->parent, UL_OBJECT_CLASS_MUTEX);
    
    mutex->owner = UL_NULL;
    mutex->hold_count = 0;
    mutex->original_priority = ULOS_CONFIG_MAX_PRIORITY;
    ul_list_init(&mutex->wait_list);

    return UL_EOK;
}

ul_mutex_t* ul_mutex_create(const char *name)
{
    struct ul_mutex *self = ul_malloc(sizeof(struct ul_mutex));

    if (self == UL_NULL)
    {
        return UL_NULL;
    }
    
    ul_mutex_init(self, name);
    
    return self;
}

// 获取互斥锁
ul_ecode ul_mutex_lock(ul_mutex_t *mutex, ul_tick_t timeout)
{
    if (mutex == UL_NULL)
    {
        return UL_ERROR;
    }

    ul_base_t level = ul_hw_interrupt_disable();

    struct ul_thread *current_thread = ul_thread_self();

    // 如果当前线程已经持有锁，递归计数加1
    if (mutex->owner == current_thread)
    {
        mutex->hold_count++;
        ul_hw_interrupt_enable(level);
        return UL_EOK;
    }

    // 如果锁未被占用
    if (mutex->owner == UL_NULL)
    {
        mutex->owner = current_thread;
        mutex->hold_count = 1;
        mutex->original_priority = current_thread->current_priority;
        ul_hw_interrupt_enable(level);
        return UL_EOK;
    }

    // 锁被占用，需要等待
    if (timeout == 0)
    {
        ul_hw_interrupt_enable(level);
        return UL_ETIMEOUT;
    }

    // 优先级继承：如果当前线程优先级高于锁持有者，提升锁持有者优先级
    if (current_thread->current_priority < mutex->owner->current_priority)
    {
        // 从就绪队列中移除锁持有者
        _thread_remove_ready_list(mutex->owner);
        // 更新优先级
        mutex->owner->current_priority = current_thread->current_priority;
        // 重新加入就绪队列
        _thread_insert_ready_list(mutex->owner);
    }

    // 将当前线程按优先级插入等待列表
    ul_list_t *pos;
    struct ul_thread *wait_thread;
    ul_bool_t inserted = UL_FALSE;

    ul_list_for_each(pos, &mutex->wait_list) {
        wait_thread = ul_list_entry(pos, struct ul_thread, ipc_list);
        if (current_thread->current_priority < wait_thread->current_priority) {
            ul_list_insert_before(pos, &current_thread->ipc_list);
            inserted = UL_TRUE;
            break;
        }
    }

    if (!inserted) {
        ul_list_insert_before(&mutex->wait_list, &current_thread->ipc_list);
    }
    //ul_list_insert_before(&mutex->wait_list, &current_thread->ipc_list);

    // 设置超时
    if (timeout != ULOS_MAX_DELAY)
    {
        current_thread->wake_tick = ulOS_get_tick() + timeout;
        ul_thread_delay(timeout);
    }
    else
    {
        ul_thread_suspend(current_thread);
    }

    ul_hw_interrupt_enable(level);

    // 被唤醒后检查是否成功获取锁
    if (mutex->owner == current_thread)
    {
        mutex->hold_count = 1;
        mutex->original_priority = current_thread->current_priority;
        return UL_EOK;
    }

    return UL_ETIMEOUT;
}

// 释放互斥锁
ul_ecode ul_mutex_unlock(ul_mutex_t *mutex)
{
    if (mutex == UL_NULL)
    {
        return UL_ERROR;
    }

    ul_base_t level = ul_hw_interrupt_disable();
    
    ul_bool_t need_schedule = UL_FALSE;
    struct ul_thread *current_thread = ul_thread_self();

    if (mutex->owner != current_thread)
    {
        return UL_ERROR;
    }

    // 递归计数减1
    mutex->hold_count--;

    // 如果计数不为0，直接返回
    if (mutex->hold_count > 0)
    {
        ul_hw_interrupt_enable(level);
        return UL_EOK;
    }

    // 恢复原始优先级
    if (current_thread->current_priority != mutex->original_priority)
    {
        _thread_remove_ready_list(current_thread);
        current_thread->current_priority = mutex->original_priority;
        _thread_insert_ready_list(current_thread);
    }

    // 如果有等待的线程，唤醒最高优先级的线程
    if (!ul_list_is_empty(&mutex->wait_list))
    {
        struct ul_thread *next_thread = ul_list_entry(mutex->wait_list.next,
                                        struct ul_thread, ipc_list);
        ul_list_del_init(&next_thread->ipc_list);
        _thread_remove_ready_list(next_thread);
        mutex->owner = next_thread;
        mutex->hold_count = 1;
        mutex->original_priority = next_thread->current_priority;

        _thread_insert_ready_list(next_thread);
        
        need_schedule = UL_TRUE;
    }
    else
    {
        mutex->hold_count = 0;
        mutex->owner = UL_NULL;
        mutex->original_priority = 255;
    }

    ul_hw_interrupt_enable(level);

    // 触发调度
    if (need_schedule == UL_TRUE)
    ul_schedule();

    return UL_EOK;
}

ul_mutex_t* ul_mutex_find(const char *name)
{
    return (ul_mutex_t*)ul_object_find(name, UL_OBJECT_CLASS_MUTEX);
}
