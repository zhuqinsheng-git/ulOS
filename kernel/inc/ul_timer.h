/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-7     zhuqinsheng   the first version
 */
#ifndef _UL_TIMER_H
#define _UL_TIMER_H

#include "ul_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 定时器命令类型 */
typedef enum
{
    ULOS_TIMER_CMD_START = 0,     /* 启动定时器 */
    ULOS_TIMER_CMD_STOP,          /* 停止定时器 */
    ULOS_TIMER_CMD_CHANGE_PERIOD, /* 改变周期 */
    ULOS_TIMER_CMD_RESET,         /* 重置定时器 */
    ULOS_TIMER_CMD_DELETE         /* 删除定时器 */
} ul_timer_cmd_t;

/* 定时器类型定义 */
#define ULOS_TIMER_TYPE_ONESHOT     0x00    /* 单次定时器 */
#define ULOS_TIMER_TYPE_PERIODIC    0x01    /* 周期性定时器 */

/* 定时器状态定义 */
#define ULOS_TIMER_STAT_STOPPED     0x00    /* 停止状态 */
#define ULOS_TIMER_STAT_STARTED     0x01    /* 启动状态 */

/**
 * @brief 定时器回调函数类型
 */
typedef void (*ul_timer_callback_t)(void *parameter);

/**
 * @brief 定时器控制块
 */
typedef struct ul_timer
{
    ul_object_t parent;                /* 继承自对象基类 */
    ul_list_t node;                    /* 链表节点 */

    ul_tick_t timeout_tick;            /* 超时节拍数 */
    ul_tick_t init_tick;              /* 初始节拍数 */

    ul_uint8_t type;            /* 定时器类型 */
    ul_uint8_t stat;           /* 定时器状态 */

    ul_timer_callback_t callback;     /* 定时器回调函数 */
    void *parameter;                  /* 回调函数参数 */
} ul_timer_t;

/* ==================== 定时器操作函数 ==================== */

/**
 * @brief 初始化定时器
 */
ul_ecode ul_timer_init(ul_timer_t *timer,
                       const char *name,
                       ul_timer_callback_t callback,
                       void *parameter,
                       ul_tick_t timeout,
                       ul_uint8_t type);

/**
 * @brief 创建定时器
 */
ul_timer_t* ul_timer_create(const char *name,
                            ul_timer_callback_t callback,
                            void *parameter,
                            ul_tick_t timeout,
                            ul_uint8_t type);

/**
 * @brief 删除定时器
 */
ul_ecode ul_timer_delete(ul_timer_t *timer);

/**
 * @brief 启动定时器
 */
ul_ecode ul_timer_start(ul_timer_t *timer);

/**
 * @brief 停止定时器
 */
ul_ecode ul_timer_stop(ul_timer_t *timer);

/**
 * @brief 重置定时器
 */
ul_ecode ul_timer_reset(ul_timer_t *timer);

/**
 * @brief 改变定时器周期
 */
ul_ecode ul_timer_change_period(ul_timer_t *timer, ul_tick_t new_period);

/**
 * @brief 初始化定时器服务
 */
ul_ecode ul_timer_thread_create(void);

#ifdef __cplusplus
}

#endif

#endif /* ULOS_TIMER_H */
