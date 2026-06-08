/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-4     zhuqinsheng   the first version
 * 2025-12-12    zhuqinsheng   优化调度算法
 */
#include "ul_thread.h"
#include "ul_libc.h"
#include "ul_idle.h"
#include "ul_debug.h"

/* ==================== 全局变量定义 ==================== */
static volatile ul_tick_t ulOS_tick = 0;                    // 系统时钟计数
static volatile ul_uint8_t ulOS_start_flag = 0;        // 系统启动标志
struct ul_thread *ul_current_thread;                  // 当前运行线程指针
ul_uint8_t ul_current_highest_priority = ULOS_CONFIG_MAX_PRIORITY - 1;  // 当前最高优先级
static ul_base_t ul_scheduler_lock_count = 0;          // 调度器锁计数器
ul_list_t ul_ready_thread_list[ULOS_CONFIG_MAX_PRIORITY];  // 就绪线程队列数组
static ul_list_t ul_delay_thread_list;                       // 延时线程队列
static ul_tick_t ul_next_wake_time = ULOS_MAX_TICK;          // 下一个唤醒时间

#if ( ULOS_CONFIG_SCHED_ALG_FFS == 1 )
volatile ul_uint32_t ul_thread_ready_priority_group = 0;

const static ul_uint8_t __lowest_bit_bitmap[] =
{
    /* 00 */ 0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 10 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 20 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 30 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 40 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 50 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 60 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 70 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 80 */ 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 90 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* A0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* B0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* C0 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* D0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* E0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* F0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};
#endif /* ULOS_CONFIG_SCHED_ALG_FFS == 1 */

/* 外部函数声明 */
extern void ul_hw_context_switch(ul_uint32_t from, ul_uint32_t to);
extern void ul_hw_context_switch_first(ul_uint32_t to);
extern ul_base_t ul_hw_interrupt_disable(void);
extern void ul_hw_interrupt_enable(ul_base_t level);
extern ul_uint8_t *ul_hw_stack_init(void *tentry, void *parameter,
                                    ul_uint8_t *stack_addr, void *texit);
extern ul_list_t ul_defunct_thread_list;

/* ==================== 线程管理相关函数 ==================== */

/**
 * @brief 线程退出处理函数
 */
static void thread_exit_entry(void)
{
    // 从就绪队列和IPC队列中移除
    ul_list_del_init(&ul_current_thread->tlist);
    ul_list_del_init(&ul_current_thread->ipc_list);

    // 加入到待删除线程列表
    ul_list_insert_before(&ul_defunct_thread_list, &ul_current_thread->tlist);

    // 更新线程状态
    ul_current_thread->state = UL_THREAD_STATE_CLOSE;

    // 触发调度
    ul_schedule();
}

/**
 * @brief 初始化线程
 */
ul_ecode ul_thread_init(struct ul_thread *self,
                        const char *name,
                        void (*entry)(void *p),
                        void *parameter,
                        void *stack_start,
                        ul_size_t stack_size,
                        ul_uint8_t priority,
                        ul_uint8_t time_slice)
{
    if (self == UL_NULL)
    {
        return UL_ENULL;
    }

    // 初始化线程对象
    ul_object_init(name, &self->parent, UL_OBJECT_CLASS_THREAD);

    // 设置线程基本属性
    self->stack_start = stack_start;
    self->stack_size = stack_size;
    self->entry = entry;
    self->parameter = parameter;
    self->current_priority = priority;
    self->init_priority = priority;
    self->state = UL_THREAD_STATE_INIT;
    self->remaining_tick = time_slice;
    self->init_tick = time_slice;

    // 初始化链表节点
    ul_list_init(&self->tlist);
    ul_list_init(&self->ipc_list);

    // 初始化栈空间
    ul_memset(self->stack_start, '#', self->stack_size);

    // 初始化栈顶指针
    self->stack_top = (void *)ul_hw_stack_init(self->entry, self->parameter,
                      (ul_uint8_t *)((char *)self->stack_start + self->stack_size),
                      (void *)thread_exit_entry);

    return UL_EOK;
}

/**
 * @brief 创建线程
 */
ul_thread_t *ul_thread_create(const char *name,
                              void (*entry)(void *p),
                              void *parameter,
                              ul_size_t stack_size,
                              ul_uint8_t priority,
                              ul_uint8_t time_slice)
{
    // 分配线程控制块
    struct ul_thread *self = ul_malloc(sizeof(struct ul_thread));

    if (self == UL_NULL)
    {
        return UL_NULL;
    }

    // 分配栈空间
    void *stack = ul_malloc(stack_size);

    if (stack == UL_NULL)
    {
        ul_free(self);
        return UL_NULL;
    }

    // 初始化线程
    ul_thread_init(self, name, entry, parameter, stack, stack_size, priority, time_slice);

    return self;
}

/**
 * @brief 删除线程
 */
void ul_thread_delete(ul_thread_t *thread)
{
    if (thread == UL_NULL)
    {
        return;
    }

    // 从队列中移除
    ul_list_del_init(&thread->tlist);
    ul_list_del_init(&thread->ipc_list);

    // 加入待删除列表
    ul_list_insert_before(&ul_defunct_thread_list, &thread->tlist);

    // 更新状态
    thread->state = UL_THREAD_STATE_CLOSE;
}

/**
 * @brief 获取当前线程
 */
ul_thread_t *ul_thread_self(void)
{
    return ul_current_thread;
}

/**
 * @brief 获取线程剩余栈空间
 */
ul_size_t ul_thread_stack_remain(ul_thread_t *thread)
{
    if (thread == UL_NULL || thread->stack_start == UL_NULL)
    {
        return 0;
    }

    ul_uint8_t *stack_bottom = (ul_uint8_t *)thread->stack_start;
    ul_size_t remain_size = 0;

    // 从栈底开始统计连续的'#'字符
    while (*stack_bottom == '#' &&
            (ul_size_t)stack_bottom < (ul_size_t)thread->stack_start + thread->stack_size)
    {
        remain_size++;
        stack_bottom++;
    }

    return remain_size;
}

/**
 * @brief 查找线程对象
 */
ul_thread_t *ul_thread_find(const char *name)
{
    return (ul_thread_t*)ul_object_find(name, UL_OBJECT_CLASS_THREAD);
}

/* ==================== 调度器相关函数 ==================== */

/**
 * @brief 初始化调度器
 */
void ul_scheduler_init(void)
{
    // 初始化就绪队列
    for (int i = 0; i < ULOS_CONFIG_MAX_PRIORITY; i++)
    {
        ul_list_init(&ul_ready_thread_list[i]);
    }

    // 初始化延时队列
    ul_list_init(&ul_delay_thread_list);
}

#if ( ULOS_CONFIG_SCHED_ALG_FFS == 1 )
static ul_uint8_t _ul_ffs(ul_uint32_t value)
{
    if (value == 0) return ULOS_CONFIG_MAX_PRIORITY + 1;// 表示空闲线程在运行

    if (value & 0xff)
        return __lowest_bit_bitmap[value & 0xff] + 1;

    if (value & 0xff00)
        return __lowest_bit_bitmap[(value & 0xff00) >> 8] + 9;

    if (value & 0xff0000)
        return __lowest_bit_bitmap[(value & 0xff0000) >> 16] + 17;

    return __lowest_bit_bitmap[(value & 0xff000000) >> 24] + 25;
}

#endif /* ULOS_CONFIG_SCHED_ALG_FFS == 1 */

/**
 * @brief 查找最高优先级
 * @return 最高优先级值
 */
static ul_uint8_t ul_find_highest_priority(void)
{

    // 从最高优先级(0)开始查找
    for (ul_uint8_t priority = 0; priority < ULOS_CONFIG_MAX_PRIORITY; priority++)
    {
        if (!ul_list_is_empty(&ul_ready_thread_list[priority]))
        {
            return priority;
        }
    }

    return ULOS_CONFIG_MAX_PRIORITY;  // 空闲任务在运行
}

/**
 * @brief 将线程插入就绪队列
 */
void _thread_insert_ready_list(struct ul_thread *thread)
{
    // 确保线程已从原队列移除
    if (!ul_list_is_empty(&thread->tlist))
    {
        ul_list_del_init(&thread->tlist);
    }

    // 插入对应优先级队列
    ul_uint8_t priority = thread->current_priority;
    ul_list_insert_before(&ul_ready_thread_list[priority], &thread->tlist);
    thread->state = UL_THREAD_STATE_READY;

#if (ULOS_CONFIG_SCHED_ALG_FFS == 1)
    /* ---------- FFS位图算法 ---------- */
    ul_thread_ready_priority_group |= (1UL << thread->current_priority);
    ul_current_highest_priority = _ul_ffs(ul_thread_ready_priority_group) - 1;
#else

    /* ---------- 轮询查找算法 ---------- */
    // 只要新线程优先级 < 当前记录的最高优先级，
    if (priority < ul_current_highest_priority)
    {
        ul_current_highest_priority = ul_find_highest_priority();
    }

#endif /* ULOS_CONFIG_SCHED_ALG_FFS == 1 */
}

/**
 * @brief 从就绪队列移除线程
 */
void _thread_remove_ready_list(struct ul_thread *thread)
{
    ul_uint8_t priority = thread->current_priority;
    ul_list_del_init(&thread->tlist);
    thread->state &= (~UL_THREAD_STATE_READY);

#if (ULOS_CONFIG_SCHED_ALG_FFS == 1)

    /* ---------- FFS位图算法 ---------- */
    if (ul_list_is_empty(&ul_ready_thread_list[priority]))
    {
        ul_thread_ready_priority_group &= ~(1UL << priority);
    }

    if (priority == ul_current_highest_priority)
    {
        ul_current_highest_priority = _ul_ffs(ul_thread_ready_priority_group) - 1;
    }

#else

    /* ---------- 轮询查找算法 ---------- */
    // 只有当移除的线程属于当前记录的最高优先级，且该优先级列表变空时，才需要查找新的最高优先级
    if ((priority == ul_current_highest_priority) &&
            ul_list_is_empty(&ul_ready_thread_list[priority]))
    {
        ul_current_highest_priority = ul_find_highest_priority();
    }

#endif /* ULOS_CONFIG_SCHED_ALG_FFS == 1 */
}

/**
 * @brief 启动调度器
 */
void ul_scheduler_start(void)
{
    struct ul_thread *to_thread;

    UL_ASSERT(ul_current_highest_priority == ul_find_highest_priority());
    to_thread = ul_list_entry(ul_ready_thread_list[ul_current_highest_priority].next,
                              struct ul_thread, tlist);

    // 设置当前任务
    ul_current_thread = to_thread;
    _thread_remove_ready_list(to_thread);
    to_thread->state = UL_THREAD_STATE_RUNNING;
    // 启动调度
    ul_hw_interrupt_disable();
    ulOS_start_flag = 1;
    ul_hw_context_switch_first((ul_uint32_t)&to_thread->stack_top);
}

/**
 * @brief 任务调度函数
 */
void ul_schedule(void)
{
    struct ul_thread *to_thread;
    struct ul_thread *from_thread;

    // 检查调度器锁
    if (ul_scheduler_lock_count > 0)
    {
        return;
    }

    // 关中断
    ul_base_t level = ul_hw_interrupt_disable();

    // 查找最高优先级任务
    UL_ASSERT(ul_current_highest_priority == ul_find_highest_priority());

    if (ul_current_highest_priority == ULOS_CONFIG_MAX_PRIORITY)    // 只有空闲任务在运行，不用切换
    {
        ul_hw_interrupt_enable(level);
        return;
    }

    // 获取要切换的任务
    to_thread = ul_list_entry(ul_ready_thread_list[ul_current_highest_priority].next,
                              struct ul_thread, tlist);

    // 如果是当前任务，不需要切换
    if (to_thread == ul_current_thread)
    {
        ul_hw_interrupt_enable(level);
        return;
    }

    // 处理当前任务，这里要改，如果是因为优先级被切换的，不能重新放最后？不一定
    if (ul_current_thread->state == UL_THREAD_STATE_RUNNING)
    {
        _thread_remove_ready_list(ul_current_thread);
        _thread_insert_ready_list(ul_current_thread);
    }

    // 切换上下文
    from_thread = ul_current_thread;
    ul_current_thread = to_thread;
    _thread_remove_ready_list(to_thread);
    to_thread->state = UL_THREAD_STATE_RUNNING;

    ul_hw_context_switch((ul_uint32_t)&from_thread->stack_top,
                         (ul_uint32_t)&to_thread->stack_top);
    ul_hw_interrupt_enable(level);
}

/* ==================== 线程控制相关函数 ==================== */

/**
 * @brief 启动线程
 */
ul_ecode ul_thread_startup(struct ul_thread *self)
{
    if (self == UL_NULL)
    {
        return UL_ENULL;
    }

    // 加入就绪队列
    _thread_insert_ready_list(self);
    return UL_EOK;
}

/**
 * @brief 设置线程优先级
 */
ul_ecode ul_thread_control_set_priority(struct ul_thread *thread, ul_uint8_t priority)
{
    if (thread == UL_NULL)
    {
        return UL_ENULL;
    }

    if (priority >= ULOS_CONFIG_MAX_PRIORITY)
    {
        return UL_ERROR;
    }

    ul_base_t level = ul_hw_interrupt_disable();

    // 更新优先级
    thread->current_priority = priority;

    // 如果线程正在运行或就绪，重新插入队列
    if (thread->state & UL_THREAD_STATE_READY)
    {
        _thread_remove_ready_list(thread);
        _thread_insert_ready_list(thread);

        // 如果是当前运行线程，且新优先级比当前最高优先级低，触发调度
        if (thread == ul_current_thread &&
                priority > ul_current_highest_priority)
        {
            ul_hw_interrupt_enable(level);
            ul_schedule();
            return UL_EOK;
        }
    }

    ul_hw_interrupt_enable(level);
    return UL_EOK;
}

/**
 * @brief 获取线程优先级
 */
ul_uint8_t ul_thread_control_get_priority(struct ul_thread *thread)
{
    if (thread == UL_NULL)
    {
        return ULOS_CONFIG_MAX_PRIORITY;  // 返回无效优先级
    }

    return thread->current_priority;
}

/**
 * @brief 恢复线程到初始优先级
 */
ul_ecode ul_thread_control_restore_priority(struct ul_thread *thread)
{
    if (thread == UL_NULL)
    {
        return UL_ENULL;
    }

    return ul_thread_control_set_priority(thread, thread->init_priority);
}

/* ==================== 延时相关函数 ==================== */

/**
 * @brief 更新下一个唤醒时间
 */
void _update_next_wake_time(void)
{
    struct ul_thread *thread = UL_NULL;
    ul_list_t *first_block_thread_node = UL_NULL;

    first_block_thread_node = ul_delay_thread_list.next;

    if (first_block_thread_node == &ul_delay_thread_list)
    {
        ul_next_wake_time = ULOS_MAX_TICK;
        return;
    }

    thread = ul_list_entry(first_block_thread_node, struct ul_thread, tlist);
    ul_next_wake_time = thread->wake_tick;
}

/**
 * @brief 线程延时
 */
void ul_thread_delay(ul_tick_t xTicksToDelay)
{
    struct ul_thread *pxCurrentTCB = ul_current_thread;
    ul_base_t level;
    ul_tick_t xTimeToWake;

    level = ul_hw_interrupt_disable();

    // 计算唤醒时间
    xTimeToWake = (xTicksToDelay == ULOS_MAX_DELAY) ?
                  ULOS_MAX_DELAY : ulOS_tick + xTicksToDelay;
    pxCurrentTCB->wake_tick = xTimeToWake;

    // 从就绪队列移除
    _thread_remove_ready_list(pxCurrentTCB);

    // 按唤醒时间插入延时队列
    ul_list_t *pxIterator;
    struct ul_thread *pxNextTCB;
    ul_bool_t inserted = UL_FALSE;

    ul_list_for_each(pxIterator, &ul_delay_thread_list)
    {
        pxNextTCB = ul_list_entry(pxIterator, struct ul_thread, tlist);

        if (pxNextTCB->wake_tick > xTimeToWake)
        {
            ul_list_insert_before(pxIterator, &pxCurrentTCB->tlist);
            inserted = UL_TRUE;
            break;
        }
    }

    if (!inserted)
    {
        ul_list_insert_before(&ul_delay_thread_list, &pxCurrentTCB->tlist);
    }

    _update_next_wake_time();
    pxCurrentTCB->state = UL_THREAD_STATE_BLOCK;

    ul_hw_interrupt_enable(level);
    ul_schedule();
}

/**
 * @brief 精确延时
 */
void ul_thread_delay_until(ul_tick_t *const pxPreviousWakeTime, const ul_tick_t xTimeIncrement)
{
    ul_tick_t xTimeToWake = *pxPreviousWakeTime + xTimeIncrement;
    *pxPreviousWakeTime = xTimeToWake;

    ul_base_t level = ul_hw_interrupt_disable();
    struct ul_thread *thread = ul_current_thread;

    thread->wake_tick = xTimeToWake;
    _thread_remove_ready_list(thread);

    // 插入延时队列
    ul_list_t *iterator;
    struct ul_thread *next_thread;
    ul_bool_t inserted = UL_FALSE;

    ul_list_for_each(iterator, &ul_delay_thread_list)
    {
        next_thread = ul_list_entry(iterator, struct ul_thread, tlist);

        if (next_thread->wake_tick > xTimeToWake)
        {
            ul_list_insert_before(iterator, &thread->tlist);
            inserted = UL_TRUE;
            break;
        }
    }

    if (!inserted)
    {
        ul_list_insert_before(&ul_delay_thread_list, &thread->tlist);
    }

    _update_next_wake_time();
    thread->state = UL_THREAD_STATE_SUSPEND;
    ul_schedule();
    ul_hw_interrupt_enable(level);
}

/**
 * @brief 挂起线程
 */
void ul_thread_suspend(struct ul_thread *self)
{
    if (self == UL_NULL)
    {
        return;
    }

    ul_base_t level = ul_hw_interrupt_disable();

    // 根据线程状态进行不同处理
    if (self->state & UL_THREAD_STATE_READY)
    {
        ul_list_del_init(&self->tlist);
        self->state = UL_THREAD_STATE_SUSPEND;
    }
    else if (self->state & UL_THREAD_STATE_BLOCK)
    {
        ul_list_del_init(&self->tlist);
        self->state = UL_THREAD_STATE_SUSPEND;
        _update_next_wake_time();
    }
    else if (self->state & UL_THREAD_STATE_RUNNING)
    {
        self->state = UL_THREAD_STATE_SUSPEND;
        ul_hw_interrupt_enable(level);
        ul_schedule();
        return;
    }

    ul_hw_interrupt_enable(level);
}

/**
 * @brief 恢复线程
 */
void ul_thread_resume(struct ul_thread *self)
{
    if (self == UL_NULL)
    {
        return;
    }

    ul_base_t level = ul_hw_interrupt_disable();

    if (self->state == UL_THREAD_STATE_SUSPEND)
    {
        _thread_insert_ready_list(self);
        self->state = UL_THREAD_STATE_READY;
    }

    ul_hw_interrupt_enable(level);
}

/* ==================== 系统时钟相关函数 ==================== */

/**
 * @brief 系统时钟中断处理函数
 */
void ul_tick_increase(void)
{
    if (ulOS_start_flag == 0)
    {
        return;
    }

    ulOS_tick++;

#if (ULOS_CONFIG_USE_TICKHOOK == 1)
    extern void ulOS_tick_hook(void);
    ulOS_tick_hook();
#endif

    struct ul_thread *thread = ul_current_thread;
    thread->remaining_tick--;

    // 检查延时任务
    if (ulOS_tick >= ul_next_wake_time)
    {
        struct ul_thread *pxTCB;
        ul_list_t *pxIterator, *pxNext;

        ul_list_for_each_safe(pxIterator, pxNext, &ul_delay_thread_list)
        {
            pxTCB = ul_list_entry(pxIterator, struct ul_thread, tlist);

            if (pxTCB->wake_tick <= ulOS_tick)
            {
                ul_list_del_init(&pxTCB->tlist);
                _thread_insert_ready_list(pxTCB);
            }
            else
            {
                ul_next_wake_time = pxTCB->wake_tick;
                break;
            }
        }

        if (ul_list_is_empty(&ul_delay_thread_list))
        {
            ul_next_wake_time = ULOS_MAX_TICK;
        }
    }

    // 检查是否需要调度

    UL_ASSERT(ul_current_highest_priority == ul_find_highest_priority());

    if (thread->remaining_tick == 0 || ul_current_highest_priority < thread->current_priority)
    {
        if (thread->remaining_tick == 0)
        {
            thread->remaining_tick = thread->init_tick;
        }

        ul_schedule();
    }
}

/* ==================== 临界区相关函数 ==================== */

/**
 * @brief 进入临界区
 */
void ul_enter_critical(void)
{
    ul_base_t level = ul_hw_interrupt_disable();
    ul_scheduler_lock_count++;
    ul_hw_interrupt_enable(level);
}

/**
 * @brief 退出临界区
 */
void ul_exit_critical(void)
{
    ul_base_t level = ul_hw_interrupt_disable();

    if (ul_scheduler_lock_count > 0)
    {
        ul_scheduler_lock_count--;

        if (ul_scheduler_lock_count == 0)
        {
            if (ul_current_thread->state == UL_THREAD_STATE_RUNNING)
            {
                ul_schedule();
            }
        }
    }

    ul_hw_interrupt_enable(level);
}

/* ==================== 系统初始化相关函数 ==================== */

/**
 * @brief 获取系统时钟
 */
ul_tick_t ulOS_get_tick(void)
{
    return ulOS_tick;
}

#if ( ULOS_CONFIG_USE_TOPIC == 1 )
#include "ul_topic.h"
static struct ul_thread *topic_thread_handle;

void topic_thread_entry(void *p)
{
    while (1)
    {
        ul_topic_process_all();
        ul_thread_delay(5);
    }
}

#endif

/**
 * @brief 内核初始化
 */
void ul_kernel_init(void)
{
    // 初始化调度器
    ul_scheduler_init();

    // 创建空闲线程
    ul_idle_thread_create();

#if ( ULOS_CONFIG_USE_TOPIC == 1 )
    // 创建主题处理线程
    static struct ul_thread *topic_thread_handle;
    topic_thread_handle = ul_thread_create("topic",
                                           topic_thread_entry,
                                           NULL,
                                           256,
                                           ULOS_CONFIG_MAX_PRIORITY - 2,
                                           1);
    ul_thread_startup(topic_thread_handle);
#endif

#if ( ULOS_CONFIG_USE_TIMER == 1 )
    extern ul_ecode ul_timer_thread_create(void);
    ul_timer_thread_create();
#endif /* ULOS_CONFIG_USE_TIMER == 1 */
}
