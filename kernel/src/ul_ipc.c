/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-4     zhuqinsheng   the first version
 */
#include "ul_ipc.h"
#include "ul_libc.h"
#include "ul_thread.h"
#include "ul_debug.h"

extern ul_base_t ul_hw_interrupt_disable(void);
extern void ul_hw_interrupt_enable(ul_base_t level);
extern void _thread_insert_ready_list(struct ul_thread *thread);
extern void _thread_remove_ready_list(struct ul_thread *thread);
extern void _update_next_wake_time(void);

/* 线程唤醒函数 */
static void _ipc_wakeup_first_receiver(ul_ipc_object_t *ipc, ul_thread_t *sender)
{
    ul_thread_t *thread;
    
    if (!ul_list_is_empty(&ipc->suspend_thread_list))
    {
        /* 获取最高优先级的等待线程 */
        thread = ul_list_entry(ipc->suspend_thread_list.next,
                              ul_thread_t,
                              ipc_list);
        ul_list_del_init(&thread->ipc_list);
        
        thread->err = UL_EOK;
        
        
        if (thread == ul_thread_self()) // 防止是自己发给自己，重要
        {
            return;
            
        }
        
        if (thread->state == UL_THREAD_STATE_BLOCK) // 如果是在阻塞过程中恢复，需要更新时间
        {
            ul_list_del_init(&thread->tlist);
            _update_next_wake_time();
        }
        
        /* 恢复到就绪列表 */
        _thread_insert_ready_list(thread);
        
        /* 检查是否需要调度 */
        if (thread->current_priority < sender->current_priority)
        {
            ul_schedule();  /* 高优先级任务被唤醒，立即调度 */
        }
    }
}

#if ULOS_CONFIG_USE_QUEUE != 0
ul_ecode ul_queue_init(ul_queue_t *queue, 
                        const char *name,
                        void *buffer,
                        uint16_t capacity,
                        uint16_t msg_size)
{
    UL_ASSERT(queue != NULL);
    UL_ASSERT(name != NULL);
    UL_ASSERT(capacity > 0);
    UL_ASSERT(msg_size > 0);

    if (queue->buffer == NULL)
    {
        return UL_ENULL;
    }
    
    /* 初始化对象基类 */
    ul_object_init(name, &queue->parent.parent, UL_OBJECT_CLASS_QUEUE);

    /* 分配队列缓冲区 */
    queue->buffer = buffer;
    
    /* 初始化队列参数 */
    queue->msg_size = msg_size;
    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    /* 只初始化接收等待列表 */
    ul_list_init(&queue->parent.suspend_thread_list);

    return UL_EOK;
}

ul_queue_t* ul_queue_create(const char *name,
                        uint16_t capacity,
                        uint16_t msg_size)
{
    UL_ASSERT(name != NULL);
    UL_ASSERT(capacity > 0);
    UL_ASSERT(msg_size > 0);

    struct ul_queue *queue;
    
    queue = ul_malloc(sizeof(struct ul_queue));
    if (queue == NULL)
    {
        return UL_NULL;
    }
    
    /* 分配队列缓冲区 */
    queue->buffer = ul_malloc(capacity * msg_size);
    if (queue->buffer == NULL)
    {
        return UL_NULL;
    }
    
    ul_queue_init(queue, name, queue->buffer, capacity, msg_size);
    
    return queue;
}

ul_ecode ul_queue_delete(ul_queue_t *queue)
{
    ul_thread_t *thread;
    uint32_t level;
    
    UL_ASSERT(queue != NULL);
    
    level = ul_hw_interrupt_disable();
    
    /* 唤醒所有等待的接收线程 */
    while (!ul_list_is_empty(&queue->parent.suspend_thread_list))
    {
        thread = ul_list_entry(queue->parent.suspend_thread_list.next,
                              ul_thread_t,
                              ipc_list);
        ul_list_del_init(&thread->ipc_list);
        
        /* 设置错误码并恢复线程 */
        _thread_insert_ready_list(thread);
    }
    
    /* 如果有线程被唤醒且优先级更高，触发调度 */
    //ul_schedule_if_needed();
    
    ul_hw_interrupt_enable(level);

    /* 释放缓冲区 */
    if (queue->buffer != NULL)
    {
        ul_free(queue->buffer);
        queue->buffer = NULL;
    }

    /* 脱离对象容器 */
    ul_object_unregister(&queue->parent.parent);

    return UL_EOK;
}



ul_ecode ul_queue_send(ul_queue_t *queue,
                      const void *buffer,
                      ul_uint16_t len,
                      ul_bool_t overwrite)
{
    ul_thread_t *current_thread;
    uint32_t level;
    ul_ecode ret = UL_EOK;
    
    UL_ASSERT(queue != NULL);
    UL_ASSERT(buffer != NULL);

    level = ul_hw_interrupt_disable();
    
    current_thread = ul_thread_self();
    
    if (len > queue->msg_size)
    {
        ul_hw_interrupt_enable(level);
        return UL_ERROR;
    }
    
    /* 队列满时的处理 */
    if (queue->count == queue->capacity)
    {
        if (!overwrite)
        {
            ul_hw_interrupt_enable(level);
            return UL_EFULL;
        }
        
        /* 覆盖模式：丢弃最旧的数据 */
        queue->head = (queue->head + 1) % queue->capacity;
        queue->count--;
        ret = UL_EFULL;
    }

    /* 复制数据到队列 */
    ul_memcpy((uint8_t *)queue->buffer + queue->tail * queue->msg_size,
           buffer,
           len);

    /* 更新队列索引 */
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;

    /* 如果有接收线程在等待，唤醒第一个 */

    _ipc_wakeup_first_receiver(&queue->parent, current_thread);

    ul_hw_interrupt_enable(level);
    
    return ret;
}

ul_ecode ul_queue_send_urgent(ul_queue_t *queue,
                             const void *buffer,
                             ul_uint16_t len,
                             ul_bool_t overwrite)
{
    ul_thread_t *current_thread;
    uint32_t level;
    ul_ecode ret = UL_EOK;
    
    UL_ASSERT(queue != NULL);
    UL_ASSERT(buffer != NULL);

    level = ul_hw_interrupt_disable();
    
    current_thread = ul_thread_self();
    
    if (len > queue->msg_size)
    {
        ul_hw_interrupt_enable(level);
        return UL_ERROR;
    }
    
    /* 队列满时的处理 */
    if (queue->count == queue->capacity)
    {
        if (!overwrite)
        {
            ul_hw_interrupt_enable(level);
            return UL_EFULL;
        }
        
        /* 覆盖模式：丢弃队头最旧的数据 */
        queue->head = (queue->head + 1) % queue->capacity;
        queue->count--;
        ret = UL_EFULL;
    }

    /* 将数据插入到队列头部 */
    /* 先移动head指针 */
    queue->head = (queue->head - 1 + queue->capacity) % queue->capacity;
    
    /* 复制数据到队列头部 */
    ul_memcpy((uint8_t *)queue->buffer + queue->head * queue->msg_size,
           buffer,
           len);

    /* 更新队列计数 */
    queue->count++;

    /* 如果有接收线程在等待，唤醒第一个 */
    _ipc_wakeup_first_receiver(&queue->parent, current_thread);

    ul_hw_interrupt_enable(level);
    
    return ret;
}


ul_ecode ul_queue_receive(ul_queue_t *queue,
                         void *buffer,
                         ul_uint16_t len,
                         ul_tick_t timeout)
{
    ul_thread_t *current_thread;
    uint32_t level;
    ul_ecode ret = UL_EOK;
    
    UL_ASSERT(queue != NULL);
    UL_ASSERT(buffer != NULL);

    level = ul_hw_interrupt_disable();

    current_thread = ul_thread_self();
    
    /* 队列空，需要等待 */
    while (queue->count == 0)
    {
        /* 非阻塞模式直接返回 */
        if (timeout == 0)
        {
            
            ul_hw_interrupt_enable(level);
            return UL_EEMPTY;
        }

        
        ul_list_insert_after(&queue->parent.suspend_thread_list, &current_thread->ipc_list);
        
            // 设置超时
        if (timeout != ULOS_MAX_DELAY)
        {
            current_thread->wake_tick = ulOS_get_tick() + timeout;
            current_thread->err = UL_ERROR;
            
            ul_thread_delay(timeout);
           
            ul_hw_interrupt_enable(level);
            return UL_ETIMEOUT;
        }
        else
        {
            current_thread->err = UL_ERROR;
            
            ul_thread_suspend(current_thread);
            

        }

        ul_hw_interrupt_enable(level);
          
        /* 被唤醒后 */
        if (current_thread->err == UL_EOK)
        {
            ret = UL_EOK;
        }
        else
        {
            ret = UL_ETIMEOUT;
        }
        
        level = ul_hw_interrupt_disable();
    }

    /* 从队列复制数据 */
    ul_memcpy(buffer,
           (uint8_t *)queue->buffer + queue->head * queue->msg_size,
           len);

    /* 更新队列索引 */
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;

    ul_hw_interrupt_enable(level);

    return ret;
}

uint16_t ul_queue_available(ul_queue_t *queue)
{
    uint32_t level;
    uint16_t count;

    UL_ASSERT(queue != NULL);

    level = ul_hw_interrupt_disable();
    count = queue->count;
    ul_hw_interrupt_enable(level);

    return count;
}

uint16_t ul_queue_available_space(ul_queue_t *queue)
{
    uint32_t level;
    uint16_t space;

    UL_ASSERT(queue != NULL);

    level = ul_hw_interrupt_disable();
    space = queue->capacity - queue->count;
    ul_hw_interrupt_enable(level);

    return space;
}
#endif /* ULOS_CONFIG_USE_QUEUE != 0 */

#if ULOS_CONFIG_USE_SEMAPHORE != 0
ul_ecode ul_sem_init(ul_sem_t *self, 
                     const char *name,
                     ul_uint16_t init_count)
{
    UL_ASSERT(self != NULL);

    /* 初始化对象基类 */
    ul_object_init(name, &self->parent.parent, UL_OBJECT_CLASS_SEMAPHORE);

    self->count = init_count;

    /* 只初始化接收等待列表 */
    ul_list_init(&self->parent.suspend_thread_list);

    return UL_EOK;
}

ul_sem_t* ul_sem_create(const char *name,
                        ul_uint16_t init_count)
{

    struct ul_semaphore *self;
    
    self = ul_malloc(sizeof(struct ul_semaphore));
    if (self == NULL)
    {
        return UL_NULL;
    }
    
    ul_sem_init(self, name, init_count);
    
    return self;
}

ul_ecode ul_sem_delete(ul_sem_t *self)
{
    
    UL_ASSERT(self != NULL);
    
    ul_thread_t *thread;
    uint32_t level;
    
    level = ul_hw_interrupt_disable();
    
    /* 唤醒所有等待的接收线程 */
    while (!ul_list_is_empty(&self->parent.suspend_thread_list))
    {
        thread = ul_list_entry(self->parent.suspend_thread_list.next,
                              ul_thread_t,
                              ipc_list);
        ul_list_del_init(&thread->ipc_list);
        
        /* 设置错误码并恢复线程 */
        _thread_insert_ready_list(thread);
    }
    
    /* 如果有线程被唤醒且优先级更高，触发调度 */
    //ul_schedule_if_needed();
    
    ul_hw_interrupt_enable(level);

    /* 脱离对象容器 */
    ul_object_unregister(&self->parent.parent);

    return UL_EOK;
}



ul_ecode ul_sem_give(ul_sem_t *self)
{
    ul_thread_t *current_thread;
    uint32_t level;
    ul_ecode ret = UL_EOK;
    

    level = ul_hw_interrupt_disable();
    
    current_thread = ul_thread_self();
    
    
    /* 满时的处理 */
    if (self->count >= ULOS_SEMAPHORE_MAX_COUNT)
    {
        return UL_EFULL;
    }

    self->count++;

    /* 如果有接收线程在等待，唤醒第一个 */

    _ipc_wakeup_first_receiver(&self->parent, current_thread);

    ul_hw_interrupt_enable(level);
    
    return ret;
}

ul_ecode ul_sem_take(ul_sem_t *self,
                     ul_tick_t timeout)
{
    ul_thread_t *current_thread;
    uint32_t level;
    ul_ecode ret = UL_EOK;

    level = ul_hw_interrupt_disable();

    current_thread = ul_thread_self();
    
    /* 队列空，需要等待 */
    while (self->count == 0)
    {
        /* 非阻塞模式直接返回 */
        if (timeout == 0)
        {
            ul_hw_interrupt_enable(level);
            return UL_EEMPTY;
        }

        
        ul_list_insert_after(&self->parent.suspend_thread_list, &current_thread->ipc_list);
        
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
        
        /* 被唤醒后 */
        level = ul_hw_interrupt_disable();
    }

    self->count--;

    ul_hw_interrupt_enable(level);

    return ret;
}                       
#endif /* ULOS_CONFIG_USE_SEMAPHORE != 0 */ 

#if ( ULOS_CONFIG_USE_EVENT == 1 )

/* 检查事件是否满足条件 */
ul_bool_t _thread_event_check(ul_thread_t *thread,
                                      uint32_t event)
{
    uint32_t set = thread->event_set;
    uint8_t option = thread->event_info;

    /* 检查逻辑与 */
    if (option & UL_EVENT_FLAG_AND)
    {
        return (event & set) == set;
    }
    /* 检查逻辑或 */
    else if (option & UL_EVENT_FLAG_OR)
    {
        return (event & set) != 0;
    }

    return false;
}

/* 事件组初始化 */
ul_ecode ul_event_init(struct ul_event *event,
                      const char *name)
{
    UL_ASSERT(event != NULL);

    /* 初始化对象基类 */
    ul_object_init(name, &event->parent.parent, UL_OBJECT_CLASS_EVENT);

    /* 初始化事件值 */
    event->event_set = 0;

    /* 初始化等待列表 */
    ul_list_init(&event->parent.suspend_thread_list);

    return UL_EOK;
}

/* 创建事件组 */
ul_event_t* ul_event_create(const char *name)
{
    struct ul_event *event;

    /* 分配事件组控制块 */
    event = ul_malloc(sizeof(struct ul_event));
    if (event == NULL)
    {
        return NULL;
    }

    /* 初始化事件组 */
    if (ul_event_init(event, name) != UL_EOK)
    {
        ul_free(event);
        return NULL;
    }

    return event;
}

/* 删除事件组 */
ul_ecode ul_event_delete(struct ul_event *event)
{
    ul_thread_t *thread;
    uint32_t level;

    UL_ASSERT(event != NULL);

    level = ul_hw_interrupt_disable();

    /* 唤醒所有等待的线程 */
    while (!ul_list_is_empty(&event->parent.suspend_thread_list))
    {
        thread = ul_list_entry(event->parent.suspend_thread_list.next,
                              ul_thread_t,
                              ipc_list);
        ul_list_del_init(&thread->ipc_list);

        _thread_insert_ready_list(thread);
    }

    ul_hw_interrupt_enable(level);

    /* 释放事件组控制块 */
    ul_free(event);

    return UL_EOK;
}

/* 发送事件 */
ul_ecode ul_event_send(struct ul_event *event,
                      uint32_t set)
{
    ul_thread_t *thread;
    uint32_t level;
    ul_list_t *node;
    ul_list_t *next;

    UL_ASSERT(event != NULL);

    level = ul_hw_interrupt_disable();
    event->event_set |= set;

    /* 遍历所有等待的线程 */
    ul_list_for_each_safe(node, next, &event->parent.suspend_thread_list)
    {
        thread = ul_list_entry(node, ul_thread_t, ipc_list);
        
        /* 检查是否满足线程的等待条件 */
        if (_thread_event_check(thread, event->event_set))
        {
            /* 从等待列表移除 */
            ul_list_del_init(&thread->ipc_list);
            
            /* 唤醒线程 */

            _thread_insert_ready_list(thread);
        }
    }

    ul_hw_interrupt_enable(level);
    return UL_EOK;
}

/* 接收事件 */
ul_ecode ul_event_recv(struct ul_event *event,
                      uint32_t set,     // 要等待的事件位
                      uint8_t option,   // 选项（AND/OR/CLEAR）
                      ul_tick_t timeout,
                      uint32_t *recved) // 实际接收到的事件
{
    ul_thread_t *current_thread;
    uint32_t level;

    UL_ASSERT(event != NULL);

    level = ul_hw_interrupt_disable();
    
    current_thread = ul_thread_self();
    
    /* 设置等待事件 */
    current_thread->event_set = set;
    current_thread->event_info = option;
    
    /* 检查是否满足条件 */
    if (_thread_event_check(current_thread, event->event_set))
    {
        /* 满足条件，返回事件 */
        if (recved != NULL)
        {
            *recved = event->event_set & set;
        }

        /* 清除相应的事件位 */
        if (option & UL_EVENT_FLAG_CLEAR)
        {
            event->event_set &= ~set;
        }

        ul_hw_interrupt_enable(level);
        return UL_EOK;
    }

    /* 不满足条件，需要等待 */
    if (timeout == 0)
    {
        ul_hw_interrupt_enable(level);
        return UL_ETIMEOUT;
    }

    ul_list_insert_after(&event->parent.suspend_thread_list, &current_thread->ipc_list);
        
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
    
    /* 被唤醒后 */
    
    /* 返回接收到的事件 */
    if (recved != NULL)
    {
        *recved = event->event_set & set;
    }
    
    /* 清除相应的事件位 */
    if (current_thread->event_info & UL_EVENT_FLAG_CLEAR)
    {
        event->event_set &= ~current_thread->event_set;
    }
    
    return UL_EOK;
}


#endif /* ULOS_CONFIG_USE_EVENT != 0 */ 
