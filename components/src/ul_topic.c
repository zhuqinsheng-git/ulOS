/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-02     zhuqinsheng   the first version
 */

#include "ul_topic.h"

/* 事件池 */
static ul_topic_t *g_topicpool;
static ul_uint8_t g_topicpool_size;
static ul_uint32_t g_topicpool_mask;

/**
 * 初始化事件池
 * @param ptopicpool: 事件池指针
 * @param size: 事件池大小
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topicpool_init(ul_topic_t* self, ul_uint8_t size)
{
    if (self == UL_NULL || size == 0)
    {
        return UL_ERROR;
    }

    g_topicpool = self;
    g_topicpool_size = size;
    g_topicpool_mask = 0;
    for (int i = 0; i < g_topicpool_size; i++)
    {
        //g_topicpool[i].priv.state = topic_STATE_FREE; // 初始化为空闲状态
        ul_list_init(&g_topicpool[i].priv.subscribers);
    }

    return UL_EOK;
}

/**
 * 查找事件
 * @param name: 事件名称
 * @return 事件指针，未找到返回NULL
 */
static ul_topic_t* ul_topic_find(const char* name, ul_uint8_t* id)
{
    for (int i = 0; i < g_topicpool_size; i++)
    {
        if (strcmp(g_topicpool[i].name, name) == 0)
        {
            *id = i;
            return &g_topicpool[i];
        }
    }

    return UL_NULL;
}

/**
 * 通知事件订阅者
 * @param topic: 事件指针
 * @return UL_EOK 成功，UL_ERROR 失败
 */
static ul_ecode notify_subscribers(ul_topic_t* topic)
{
    if (!topic)
    {
        return UL_ERROR;
    }

    ul_list_t* pos;
    ul_topic_subscriber_t* subscriber;
    uint32_t notified_count = 0;

    ul_list_for_each(pos, &topic->priv.subscribers)
    {
        subscriber = ul_list_entry(pos, ul_topic_subscriber_t, priv.node);

        if (subscriber && subscriber->priv.handler)
        {
            subscriber->priv.handler(topic, topic->priv.data);
            notified_count++;
        }
    }

    // 如果没有订阅者或没有成功通知任何订阅者，返回错误
    return (notified_count > 0) ? UL_EOK : UL_ERROR;
}

/**
 * 订阅事件
 * @param topic_name: 事件名称
 * @param handler: 事件处理函数
 * @return 订阅者对象
 */
ul_topic_subscriber_t* ul_topic_subscribe(const char* topic_name, ul_topic_handler_t handler)
{
    if (!topic_name || !handler)
    {
        return UL_NULL;
    }

    // 查找topic_name对应的事件
    ul_uint8_t topic_id;
    ul_topic_t* topic = ul_topic_find(topic_name, &topic_id);

    if (!topic)
    {
        return UL_NULL;
    }

    ul_topic_subscriber_t *self = ul_malloc(sizeof(ul_topic_subscriber_t));

    if (self == UL_NULL)
    {
        return UL_NULL;
    }

    // 把self的node添加到topic_name对应的事件的订阅者列表中
    ul_list_init(&self->priv.node);
    ul_list_insert_before(&topic->priv.subscribers, &self->priv.node);

    // 设置self的handler为handler
    self->priv.handler = handler;

    return self;
}

/**
 * 取消订阅
 * @param self: 订阅者对象指针
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topic_unsubscribe(ul_topic_subscriber_t* self)
{
    if (!self)
    {
        return UL_ERROR;
    }

    // 从事件订阅列表中移除
    ul_list_del_init(&self->priv.node);

    // 释放订阅者对象
    ul_free(self);

    return UL_EOK;
}

/**
 * 清理特定事件的所有订阅者
 * @param topic_name: 事件名称
 * @return UL_EOK 成功，UL_ERROR 失败
 */
void ul_topic_cleanup_subscriptions(const char* topic_name)
{
    if (!topic_name) {
        return;
    }

    ul_uint8_t topic_id;
    ul_topic_t* topic = ul_topic_find(topic_name, &topic_id);
    if (!topic) {
        return;
    }

    // 遍历并释放所有订阅者
    ul_list_t* pos, *tmp;
    ul_topic_subscriber_t* subscriber;
    
    ul_list_for_each_safe(pos, tmp, &topic->priv.subscribers) {
        subscriber = ul_list_entry(pos, ul_topic_subscriber_t, priv.node);
        
        // 从订阅列表中移除
        ul_list_del_init(&subscriber->priv.node);
        
        // 释放订阅者对象
        ul_free(subscriber);
        subscriber = UL_NULL;
    }
}


/**
 * 发布事件
 * @param topic_name: 事件名称
 * @param is_urgent: 是否为紧急事件
 * @param data: 事件数据
 * @param data_size: 数据大小
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topic_publish(const char* topic_name, ul_uint8_t is_urgent, void* data, ul_size_t data_size)
{
    if (topic_name == UL_NULL)
    {
        return UL_ERROR;
    }

    // 查找事件
    ul_uint8_t topic_id;
    ul_topic_t* self = ul_topic_find(topic_name, &topic_id);

    if (self == UL_NULL)
    {
        return UL_ERROR;
    }

    // 更新事件数据
    if (self->priv.data != UL_NULL)
    {
        ul_free(self->priv.data);
        self->priv.data = NULL;
    }

    if ((data != UL_NULL) && data_size > 0)
    {
        self->priv.data = ul_malloc(data_size);

        if (self->priv.data == UL_NULL)
        {
            return UL_ERROR;
        }

        memcpy(self->priv.data, data, data_size);
        self->priv.data_size = data_size;
    }
    else
    {
        self->priv.data_size = 0;
    }

    // 设置事件状态和时间戳
    //self->timestamp = ul_get_tick();

    // 通知所有订阅者
    if (is_urgent == 1) //  紧急事件
    {
        notify_subscribers(self);
    }
    else
    {
        g_topicpool_mask |= (1 << topic_id);  /* 设置对应位 */
        //self->priv.state = topic_STATE_PUBLISHED;
    }

    return UL_EOK;
}




/**
 * 删除事件
 * @param topic_name: 事件名称
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topic_delete(const char* topic_name)
{
    if (!topic_name)
    {
        return UL_ERROR;
    }

    ul_uint8_t topic_id;
    ul_topic_t* topic = ul_topic_find(topic_name, &topic_id);
    if (!topic)
    {
        return UL_ERROR;
    }

    // 清理所有订阅者
    ul_list_t* pos, *tmp;
    ul_topic_subscriber_t* subscriber;
    ul_list_for_each_safe(pos, tmp, &topic->priv.subscribers)
    {
        subscriber = ul_list_entry(pos, ul_topic_subscriber_t, priv.node);
        ul_list_del_init(&subscriber->priv.node);
        ul_free(subscriber);
    }

    // 清理事件数据
    if (topic->priv.data)
    {
        ul_free(topic->priv.data);
    }

    // 重置事件状态
    //topic->priv.state = topic_STATE_FREE;
    g_topicpool_mask &= ~(1 << topic_id);
    
    return UL_EOK;
}

/**
 * 处理所有事件
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topic_process_all(void)
{
    if (g_topicpool_mask == 0)
    {
        return UL_ERROR;
    }
    
    for (ul_uint8_t i = 0; i < g_topicpool_size; i++)
    {
        if ((g_topicpool_mask & (1 << i)) != 0)
        {
            g_topicpool_mask &= ~(1 << i);
            notify_subscribers(&g_topicpool[i]);
            
            //self->priv.state = topic_STATE_FREE;
        }
    }

    return UL_EOK;
}

/**
 * 打印事件树
 * @param kprtinf: 输出函数指针
 * @return UL_EOK 成功，UL_ERROR 失败
 */
ul_ecode ul_topic_list(ul_output_func_t kprtinf)
{
    ul_topic_t *topic;
    uint8_t topic_count = 0;
    uint8_t total_subscribers = 0;

    kprtinf("\n==== UL topic Tree ====\r\n");
    kprtinf("┌─ topics\r\n");

    /* Traverse topic pool */
    for (int i = 0; i < g_topicpool_size; i++)
    {
        topic = &g_topicpool[i];

        kprtinf("│  ├─ [%u][%s] (State:%s)\r\n",
                i + 1,
                topic->name,
                (g_topicpool_mask & (1 << i)) == 0 ? "Free" :
                (g_topicpool_mask & (1 << i)) != 0 ? "Published" : "Unknown");

        // 检查是否有订阅者
        if (!ul_list_is_empty(&topic->priv.subscribers))
        {
            ul_list_t *sub_pos;
            ul_topic_subscriber_t *subscriber;
            uint32_t sub_count = 0;

            ul_list_for_each(sub_pos, &topic->priv.subscribers)
            {
                subscriber = ul_list_entry(sub_pos, ul_topic_subscriber_t, priv.node);

                if (subscriber && subscriber->priv.handler)
                {
                    sub_count++;
                    total_subscribers++;

                    if (ul_list_is_last(sub_pos, &topic->priv.subscribers))
                    {
                        kprtinf("│  │  └─ Subscriber [%u] (Handler:0x%p)\r\n",
                                sub_count, subscriber->priv.handler);
                    }
                    else
                    {
                        kprtinf("│  │  ├─ Subscriber [%u] (Handler:0x%p)\r\n",
                                sub_count, subscriber->priv.handler);
                    }
                }
            }
        }

        if ((g_topicpool_mask & (1 << i)) != 0)
        {
            topic_count++;
        }
    }

    kprtinf("│\r\n");
    kprtinf("└─ Summary\r\n");
    kprtinf("   topic Pool Size: %u\r\n", g_topicpool_size);
    kprtinf("   Total Free topics: %u\r\n", g_topicpool_size - topic_count);
    kprtinf("   Total Subscribers: %u\r\n", total_subscribers);
    kprtinf("=======================\r\n\n");

    return UL_EOK;
}
