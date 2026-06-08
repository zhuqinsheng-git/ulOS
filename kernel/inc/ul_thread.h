/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-4     zhuqinsheng   the first version
 */
#ifndef _UL_THREAD_H
#define _UL_THREAD_H

#include "ul_object.h"

/* ==================== 枚举定义 ==================== */

/**
 * @brief 线程状态枚举
 */
typedef enum
{
    UL_THREAD_STATE_INIT     = 0x00,    /* 初始化状态 */
    UL_THREAD_STATE_READY    = 0x01,    /* 就绪状态 */
    UL_THREAD_STATE_RUNNING  = 0x02,    /* 运行状态 */
    UL_THREAD_STATE_SUSPEND  = 0x04,    /* 挂起状态 */
    UL_THREAD_STATE_BLOCK    = 0x08,    /* 阻塞状态 */
    UL_THREAD_STATE_CLOSE    = 0x10,    /* 关闭状态 */
    
    /* 状态掩码 */
    UL_THREAD_STATE_MASK     = 0x1F,    /* 所有状态位掩码 */
} ul_thread_state_t;

/* ==================== 结构体定义 ==================== */

/**
 * @brief 线程控制块结构体
 */
struct ul_thread
{
    /* 基础对象 */
    struct ul_object parent;          /* 继承自基础对象 */

    /* 链表节点 */
    ul_list_t tlist;                  /* 线程链表节点 */
    ul_list_t ipc_list;              /* IPC相关链表节点 */

    /* 栈相关 */
    void* stack_start;               /* 栈起始地址 */
    ul_size_t stack_size;            /* 栈大小 */
    void* stack_top;                 /* 栈顶指针 */

    /* 执行相关 */
    void *entry;                     /* 线程入口函数 */
    void* parameter;                 /* 入口函数参数 */

    /* 优先级相关 */
    ul_uint8_t current_priority;     /* 当前优先级 */
    ul_uint8_t init_priority;        /* 初始优先级 */

    /* 状态相关 */
    ul_thread_state_t state;         /* 线程状态 */
    ul_ecode err;
    /* 时间片相关 */
    ul_uint8_t remaining_tick;        /* 剩余时间片 */
    ul_uint8_t init_tick;            /* 初始时间片 */
    ul_tick_t wake_tick;             /* 唤醒时间戳 */

#if ( ULOS_CONFIG_USE_EVENT == 1 )
    /* 事件相关 */
    ul_uint32_t event_set;           /* 事件标志 */
    ul_uint32_t event_info;          /* 事件信息 */
#endif
};

typedef struct ul_thread ul_thread_t;

/* ==================== 线程管理函数 ==================== */

/**
 * @brief 初始化线程
 * @param self 线程控制块指针
 * @param name 线程名称
 * @param entry 线程入口函数
 * @param parameter 入口函数参数
 * @param stack_start 栈起始地址
 * @param stack_size 栈大小
 * @param priority 优先级
 * @param time_slice 时间片
 * @return 错误码
 */
ul_ecode ul_thread_init(struct ul_thread *self,
                        const char *name,
                        void (*entry)(void *p),
                        void* parameter,
                        void* stack_start,
                        ul_size_t stack_size,
                        ul_uint8_t priority,
                        ul_uint8_t time_slice);

/**
 * @brief 创建线程
 * @param name 线程名称
 * @param entry 线程入口函数
 * @param parameter 入口函数参数
 * @param stack_size 栈大小
 * @param priority 优先级
 * @param time_slice 时间片
 * @return 线程控制块指针
 */
ul_thread_t* ul_thread_create(const char *name,
                              void (*entry)(void *p),
                              void *parameter,
                              ul_size_t stack_size,
                              ul_uint8_t priority,
                              ul_uint8_t time_slice);

/**
 * @brief 获取当前线程
 * @return 当前线程控制块指针
 */
ul_thread_t *ul_thread_self(void);

/**
 * @brief 获取线程剩余栈空间
 * @param thread 线程控制块指针
 * @return 剩余栈大小
 */
ul_size_t ul_thread_stack_remain(ul_thread_t *thread);

/**
 * @brief 删除线程
 * @param thread 线程控制块指针
 */
void ul_thread_delete(ul_thread_t *thread);

/**
 * @brief 查找线程对象
 */
ul_thread_t* ul_thread_find(const char *name);
/* ==================== 线程控制函数 ==================== */

/**
 * @brief 启动线程
 * @param self 线程控制块指针
 * @return 错误码
 */
ul_ecode ul_thread_startup(struct ul_thread *self);

/**
 * @brief 设置线程优先级
 * @param thread 线程控制块指针
 * @param priority 新的优先级值
 * @return 错误码
 */
ul_ecode ul_thread_control_set_priority(struct ul_thread *thread, ul_uint8_t priority);

/**
 * @brief 获取线程优先级
 * @param thread 线程控制块指针
 * @return 线程优先级
 */
ul_uint8_t ul_thread_control_get_priority(struct ul_thread *thread);

/**
 * @brief 恢复线程到初始优先级
 * @param thread 线程控制块指针
 * @return 错误码
 */
ul_ecode ul_thread_control_restore_priority(struct ul_thread *thread);

/* ==================== 调度器函数 ==================== */

/**
 * @brief 内核初始化
 */
void ul_kernel_init(void);

/**
 * @brief 启动调度器
 */
void ul_scheduler_start(void);

/**
 * @brief 任务调度函数
 */
void ul_schedule(void);

/* ==================== 时间相关函数 ==================== */

/**
 * @brief 系统时钟中断处理函数
 */
void ul_tick_increase(void);

/**
 * @brief 获取系统时钟
 * @return 当前系统时钟值
 */
ul_tick_t ulOS_get_tick(void);

/**
 * @brief 线程延时
 * @param xTicksToDelay 延时tick数
 */
void ul_thread_delay(ul_tick_t xTicksToDelay);

/**
 * @brief 精确延时
 * @param pxPreviousWakeTime 上次唤醒时间指针
 * @param xTimeIncrement 时间增量
 */
void ul_thread_delay_until(ul_tick_t *const pxPreviousWakeTime, 
                           const ul_tick_t xTimeIncrement);

/**
 * @brief 挂起线程
 * @param self 线程控制块指针
 */
void ul_thread_suspend(struct ul_thread *self);

/**
 * @brief 恢复线程
 * @param self 线程控制块指针
 */
void ul_thread_resume(struct ul_thread *self);

/* ==================== 临界区函数 ==================== */

/**
 * @brief 进入临界区
 */
void ul_enter_critical(void);

/**
 * @brief 退出临界区
 */
void ul_exit_critical(void);

#endif /* ULOS_THREAD_H */
