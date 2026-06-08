/**
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *_
 * @file ul_topic.h
 * @brief 事件驱动框架
 * 
 * 基于发布-订阅模式的事件系统，提供了事件的创建、订阅、发布和处理功能。
 * 适用于模块间解耦、异步通信等场景。
 * 
 * 主要特性：
 * - 支持事件池管理
 * - 支持多个订阅者订阅同一事件
 * - 支持紧急事件和普通事件
 * - 支持自定义事件处理函数
 * - 支持事件数据传递
 * 
 * 使用示例：
 * @code
 * // 1. 定义事件池（最多32个事件）
 * static ul_topic_t topicpool[]=
 * {
 *     {.name = "tim.2s"},
 *     {.name = "key"},
 *     {.name = "sensor"}
 * };
 * 
 * // 2. 初始化事件池
 * ul_topicpool_init_all(topic_pool);
 * 
 * // 3. 定义事件处理函数
 * void my_topic_handler(ul_topic_t* topic, void* user_data)
 * {
 *     // 处理事件
 *     printf("subscriber xxx receive from [%s]", topic->name);
 * }
 * 
 * // 4. 订阅事件
 * ul_topic_subscriber_t* subscriber = ul_topic_subscribe("tim.2s", my_topic_handler);
 * 
 * // 5. 发布事件（可选择紧急或普通）
 * int test_data = 123;
 * ul_topic_publish("test_topic", 0, &test_data, sizeof(test_data));
 * 
 * // 6. 处理事件（RTOS放任意线程内，裸机放main的while循环内）
 * ul_topic_process_all();
 * 
 * // 7. 取消订阅
 * ul_topic_unsubscribe(subscriber);
 * @endcode
 * 
 * 注意事项：
 * 1. 使用前必须初始化事件池
 * 2. 订阅者必须提供有效的处理函数
 * 3. 事件数据的生命周期需要由发布者保证
 * 4. 建议在不需要时及时取消订阅
 * 5. 紧急事件会优先处理，若紧急事件在中断上下文内发布，订阅者的接收回调必须快进快出
 * 
 * 依赖：
 * - ul_object.h: 基础对象定义
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-02     zhuqinsheng   the first version
 */
#ifndef _UL_TOPIC_H
#define _UL_TOPIC_H

#include "ul_object.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* 事件结构 */
typedef struct ul_topic
{
    char name[UL_OBJECT_NAME_MAX_LENGTH];  /* 事件名称 */

    struct
    {
        void*               data;         /* 事件数据 */
        ul_size_t           data_size;    /* 数据大小 */
        ul_list_t           subscribers;  /* 订阅者链表 */
    }priv;
}ul_topic_t;

typedef void (*ul_topic_handler_t)(ul_topic_t* topic, void* user_data);
/* 事件订阅者结构 */
typedef struct ul_topic_subscriber
{
    struct
    {
        void*               user_data;  /* 用户数据 */
        ul_topic_handler_t  handler;    /* 处理函数 */
        ul_list_t           node;       /* 链表节点 */
    }priv;
} ul_topic_subscriber_t;

/* 计算事件池大小的宏 */
#define topicPOOL_SIZE(x)   (sizeof(x)/sizeof(x[0]))

/**
 * 初始化事件池
 * @param ptopicpool: 事件池指针
 * @param size: 事件池大小
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topicpool_init(ul_topic_t* ptopicpool, ul_uint8_t size);

/**
 * 初始化整个事件池（便捷宏）
 * @param x: 事件池数组
 * @return UL_EOK 成功，UL_ERROR 失败
 */
#define ul_topicpool_init_all(x)    ul_topicpool_init(x, topicPOOL_SIZE(x));

/**
 * 订阅事件
 * @param topic_name: 事件名称
 * @param handler: 事件处理函数
 * @return 订阅者对象指针，失败返回NULL
 */
ul_topic_subscriber_t* ul_topic_subscribe(const char* topic_name, ul_topic_handler_t handler);

/**
 * 取消订阅
 * @param self: 订阅者对象指针
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topic_unsubscribe(ul_topic_subscriber_t* self);

/**
 * 发布事件
 * @param topic_name: 事件名称
 * @param is_urgent: 是否为紧急事件（1-紧急，0-普通）
 * @param data: 事件数据
 * @param data_size: 数据大小
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topic_publish(const char* topic_name, ul_uint8_t is_urgent, void* data, ul_size_t data_size);

/**
 * 处理单个事件
 * @param self: 事件指针
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topic_process(ul_topic_t *self);

/**
 * 处理所有事件
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topic_process_all(void);

/**
 * 清理特定事件的所有订阅者
 * @param topic_name: 事件名称
 */
void ul_topic_cleanup_subscriptions(const char* topic_name);

/**
 * 打印事件树
 * @param kprtinf: 输出函数指针
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topic_list(ul_output_func_t kprtinf);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* UL_topic_H */
