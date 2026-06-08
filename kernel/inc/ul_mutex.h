/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-4     zhuqinsheng   the first version
 */
#ifndef _UL_MUTEX_H
#define _UL_MUTEX_H

#include "ul_object.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* 互斥锁结构体 */
typedef struct ul_mutex
{
    struct ul_object parent;

    struct ul_thread *owner;          /* 锁的持有者 */
    ul_uint32_t hold_count;           /* 递归持有计数 */
    ul_uint8_t original_priority;     /* 原始优先级 */
    ul_list_t wait_list;              /* 等待链表 */
} ul_mutex_t;

/* 函数声明 */

/**
 * @brief 初始化互斥锁
 * @param mutex 互斥锁控制块
 * @return UL_EOK 成功
 * @return UL_EINVAL 参数错误
 */
ul_ecode ul_mutex_init(struct ul_mutex *mutex, const char *name);
ul_mutex_t* ul_mutex_create(const char *name);
/**
 * @brief 获取互斥锁
 * @param mutex 互斥锁控制块
 * @param timeout 超时时间（单位：系统tick）
 * @return UL_EOK 成功
 * @return UL_EINVAL 参数错误
 * @return UL_ETIMEOUT 超时
 */
ul_ecode ul_mutex_lock(struct ul_mutex *mutex, ul_tick_t timeout);

/**
 * @brief 释放互斥锁
 * @param mutex 互斥锁控制块
 * @return UL_EOK 成功
 * @return UL_EINVAL 参数错误
 */
ul_ecode ul_mutex_unlock(struct ul_mutex *mutex);

ul_mutex_t* ul_mutex_find(const char *name);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
