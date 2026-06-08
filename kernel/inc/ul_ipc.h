/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-4     zhuqinsheng   the first version
 */
#ifndef _UL_IPC_H
#define _UL_IPC_H

#include "ul_object.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* ==================== IPC基础定义 ==================== */

/**
 * @brief IPC对象基类
 */
typedef struct
{
    ul_object_t parent;                /* 继承自对象基类 */
    ul_list_t suspend_thread_list;     /* 挂起线程列表 */
} ul_ipc_object_t;

/* ==================== 队列(Queue) ==================== */

#if ULOS_CONFIG_USE_QUEUE != 0

/**
 * @brief 队列控制块
 */
typedef struct ul_queue
{
    ul_ipc_object_t parent;            /* 继承自IPC对象 */
    
    void *buffer;                      /* 队列缓冲区 */
    ul_uint16_t msg_size;             /* 单个消息大小 */
    ul_uint16_t capacity;             /* 队列最大容量（消息数量）*/
    ul_uint16_t head;                 /* 队头索引 */
    ul_uint16_t tail;                 /* 队尾索引 */
    ul_uint16_t count;                /* 当前队列中的消息数量 */
} ul_queue_t;

/* ==================== 队列操作函数 ==================== */

/**
 * @brief 初始化队列
 * @param queue 队列控制块指针
 * @param name 队列名称
 * @param buffer 缓冲区指针
 * @param capacity 队列容量
 * @param msg_size 消息大小
 * @return 错误码
 */
ul_ecode ul_queue_init(ul_queue_t *queue, 
                       const char *name,
                       void *buffer,
                       uint16_t capacity,
                       uint16_t msg_size);

/**
 * @brief 删除队列
 * @param queue 队列控制块指针
 * @return 错误码
 */
ul_ecode ul_queue_delete(ul_queue_t *queue);

/**
 * @brief 创建队列
 * @param name 队列名称
 * @param capacity 队列容量
 * @param msg_size 消息大小
 * @return 队列控制块指针
 */
ul_queue_t* ul_queue_create(const char *name,
                           uint16_t capacity,
                           uint16_t msg_size);
                        
/**
 * @brief 发送消息到队列
 * @param queue 队列控制块指针
 * @param buffer 消息缓冲区
 * @param len 消息长度
 * @param overwrite 是否覆盖
 * @return 错误码
 */
ul_ecode ul_queue_send(ul_queue_t *queue,
                      const void *buffer,
                      ul_uint16_t len,
                      ul_bool_t overwrite);

/**
 * @brief 紧急发送消息到队列头
 * @param queue 队列控制块指针
 * @param buffer 消息缓冲区
 * @param len 消息长度
 * @param overwrite 是否覆盖
 * @return 错误码
 */
ul_ecode ul_queue_send_urgent(ul_queue_t *queue,
                             const void *buffer,
                             ul_uint16_t len,
                             ul_bool_t overwrite);

/**
 * @brief 从队列接收消息
 * @param queue 队列控制块指针
 * @param buffer 接收缓冲区
 * @param len 消息长度
 * @param timeout 超时时间
 * @return 错误码
 */
ul_ecode ul_queue_receive(ul_queue_t *queue,
                          void *buffer,
                          ul_uint16_t len,
                          ul_tick_t timeout);

/**
 * @brief 获取队列中可用消息数量
 * @param queue 队列控制块指针
 * @return 可用消息数量
 */
uint16_t ul_queue_available(ul_queue_t *queue);

/**
 * @brief 获取队列剩余空间
 * @param queue 队列控制块指针
 * @return 剩余空间
 */
uint16_t ul_queue_available_space(ul_queue_t *queue);

#endif /* ULOS_CONFIG_USE_QUEUE != 0 */

/* ==================== 信号量(Semaphore) ==================== */

#if ULOS_CONFIG_USE_SEMAPHORE != 0

/**
 * @brief 信号量控制块
 */
typedef struct ul_semaphore
{
    ul_ipc_object_t parent;            /* 继承自IPC对象 */
    ul_uint16_t count;                 /* 信号量计数值 */
} ul_sem_t;

/* 信号量配置 */
#ifndef ULOS_SEMAPHORE_MAX_COUNT
#define ULOS_SEMAPHORE_MAX_COUNT    ((ul_uint8_t)100)
#endif

/* ==================== 信号量操作函数 ==================== */

/**
 * @brief 初始化信号量
 * @param self 信号量控制块指针
 * @param name 信号量名称
 * @return 错误码
 */
ul_ecode ul_sem_init(ul_sem_t *self, 
                    const char *name,
                    ul_uint16_t init_count);

/**
 * @brief 创建信号量
 * @param name 信号量名称
 * @return 信号量控制块指针
 */
ul_sem_t* ul_sem_create(const char *name,
                        ul_uint16_t init_count);

/**
 * @brief 删除信号量
 * @param self 信号量控制块指针
 * @return 错误码
 */
ul_ecode ul_sem_delete(ul_sem_t *self);

/**
 * @brief 释放信号量
 * @param self 信号量控制块指针
 * @return 错误码
 */
ul_ecode ul_sem_give(ul_sem_t *self);

/**
 * @brief 获取信号量
 * @param self 信号量控制块指针
 * @param timeout 超时时间
 * @return 错误码
 */
ul_ecode ul_sem_take(ul_sem_t *self,
                    ul_tick_t timeout);

#endif /* ULOS_CONFIG_USE_SEMAPHORE != 0 */

/* ==================== 事件(Event) ==================== */

#if ULOS_CONFIG_USE_EVENT != 0

/**
 * @brief 事件组控制块
 */
typedef struct ul_event
{
    ul_ipc_object_t parent;            /* 继承自IPC对象 */
    uint32_t event_set;                /* 当前事件集合 */
} ul_event_t;

/* 事件标志选项 */
#define UL_EVENT_FLAG_AND   0x01      /* 逻辑与 */
#define UL_EVENT_FLAG_OR    0x02      /* 逻辑或 */
#define UL_EVENT_FLAG_CLEAR 0x04      /* 清除标志 */

/* ==================== 事件组操作函数 ==================== */

/**
 * @brief 初始化事件组
 * @param event 事件组控制块指针
 * @param name 事件组名称
 * @return 错误码
 */
ul_ecode ul_event_init(ul_event_t *event,
                      const char *name);

/**
 * @brief 创建事件组
 * @param name 事件组名称
 * @return 事件组控制块指针
 */
ul_event_t* ul_event_create(const char *name);

/**
 * @brief 删除事件组
 * @param event 事件组控制块指针
 * @return 错误码
 */
ul_ecode ul_event_delete(ul_event_t *event);

/**
 * @brief 发送事件
 * @param event 事件组控制块指针
 * @param set 事件标志
 * @return 错误码
 */
ul_ecode ul_event_send(ul_event_t *event,
                      uint32_t set);

/**
 * @brief 接收事件
 * @param event 事件组控制块指针
 * @param set 要等待的事件位
 * @param option 选项（AND/OR/CLEAR）
 * @param timeout 超时时间
 * @param recved 实际接收到的事件
 * @return 错误码
 */
ul_ecode ul_event_recv(ul_event_t *event,
                      uint32_t set,
                      uint8_t option,
                      ul_tick_t timeout,
                      uint32_t *recved);

#endif /* ULOS_CONFIG_USE_EVENT != 0 */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* ULOS_IPC_H */
