/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-7     zhuqinsheng   the first version
 */
#include "ul_timer.h"
#include "ul_ipc.h"
#include "ul_thread.h"

/* 定时器命令消息 */
typedef struct
{
    ul_timer_cmd_t cmd;          /* 命令类型 */
    ul_timer_t *timer;           /* 定时器句柄 */
    ul_tick_t period;           /* 新的周期（用于CHANGE_PERIOD命令） */
    ul_tick_t timeout;          /* 超时时间 */
} ul_timer_msg_t;

/* 定时器相关全局变量 */
ul_list_t ul_timer_list = UL_LIST_HEAD_INIT(ul_timer_list);
ul_thread_t *timer_thread = UL_NULL;
ul_queue_t *timer_cmd_queue = UL_NULL;

/* 定时器命令处理函数声明 */
static void ul_timer_process_cmd(ul_timer_msg_t *msg);
static void ul_timer_process_expired(void);

/**
 * @brief 将定时器按超时时间插入到链表中的正确位置
 */
static void ul_timer_insert_sorted(ul_timer_t *timer)
{
    ul_list_t *node;
    ul_timer_t *tmp;

    // !!!一定要先把自己移除，
    // 否则在下方标记处会存在自己插自己屁股后面的问题，导致链表断掉
    ul_list_del_init(&timer->node);


    /* 如果链表为空，直接插入 */
    if (ul_list_is_empty(&ul_timer_list))
    {
        ul_list_insert_before(&ul_timer_list, &timer->node);
        return;
    }

    /* 遍历链表，找到合适的插入位置 */
    ul_list_for_each(node, &ul_timer_list)
    {
        tmp = ul_list_entry(node, ul_timer_t, node);

        /* 找到第一个超时时间大于当前定时器的位置 */
        if (tmp->timeout_tick > timer->timeout_tick)
        {
            ul_list_insert_before(node, &timer->node);  // 标记，这个node可能==&timer->node
            return;
        }
    }

    /* 如果没找到合适位置，插入到链表末尾 */
    ul_list_insert_before(&ul_timer_list, &timer->node);
}

/**
 * @brief 定时器线程入口函数
 */
static void timer_thread_entry(void *parameter)
{
    ul_timer_msg_t msg;
    ul_tick_t next_expire_time;
    ul_ecode ret = UL_ERROR;

    while (1)
    {
        /* 获取下一个要触发的定时器时间 */
        next_expire_time = ULOS_MAX_TICK;

        if (!ul_list_is_empty(&ul_timer_list))
        {
            ul_timer_t *timer = ul_list_entry(ul_timer_list.next, ul_timer_t, node);
            next_expire_time = timer->timeout_tick - ulOS_get_tick();

            if (next_expire_time > ULOS_MAX_TICK)
            {
                next_expire_time = 0;  /* 防止负数 */
            }
        }

        ret = ul_queue_receive(timer_cmd_queue, &msg, sizeof(ul_timer_msg_t), next_expire_time);

        /* 等待命令或定时器到期 */
        if (ret == UL_EOK)
        {
            /* 处理命令 */
            ul_timer_process_cmd(&msg);
        }
        else if (ret == UL_ETIMEOUT)
        {
            /* 处理到期的定时器 */
            ul_timer_process_expired();
        }
    }
}

/**
 * @brief 处理定时器命令
 */
static void ul_timer_process_cmd(ul_timer_msg_t *msg)
{
    ul_timer_t *timer = msg->timer;

    switch (msg->cmd)
    {
    case ULOS_TIMER_CMD_START:
        if (timer->stat == ULOS_TIMER_STAT_STARTED)
        {
            ul_list_del_init(&timer->node);
        }

        timer->timeout_tick = msg->timeout;
        timer->stat = ULOS_TIMER_STAT_STARTED;
        /* 使用排序插入 */
        ul_timer_insert_sorted(timer);
        break;

    case ULOS_TIMER_CMD_STOP:
        if (timer->stat == ULOS_TIMER_STAT_STARTED)
        {
            ul_list_del_init(&timer->node);
            timer->stat = ULOS_TIMER_STAT_STOPPED;
        }

        break;

    case ULOS_TIMER_CMD_RESET:
        if (timer->stat == ULOS_TIMER_STAT_STARTED)
        {
            ul_list_del_init(&timer->node);
            timer->timeout_tick = msg->timeout;
            /* 使用排序插入 */
            ul_timer_insert_sorted(timer);
        }

        break;

    case ULOS_TIMER_CMD_CHANGE_PERIOD:
        timer->init_tick = msg->period;

        if (timer->stat == ULOS_TIMER_STAT_STARTED)
        {
            ul_list_del_init(&timer->node);
            timer->timeout_tick = ulOS_get_tick() + timer->init_tick;
            /* 使用排序插入 */
            ul_timer_insert_sorted(timer);
        }

        break;

    case ULOS_TIMER_CMD_DELETE:
        if (timer->stat == ULOS_TIMER_STAT_STARTED)
        {
            ul_list_del_init(&timer->node);
        }

        ul_free(timer);
        break;

    default:
        break;
    }
}

/**
 * @brief 处理到期的定时器
 */
static void ul_timer_process_expired(void)
{
    ul_list_t *node;
    ul_list_t *next;
    ul_timer_t *timer;
    ul_tick_t current_tick = ulOS_get_tick();

    /* 由于链表是按超时时间排序的，只需要处理链表前面的定时器 */
    ul_list_for_each_safe(node, next, &ul_timer_list)
    {
        timer = ul_list_entry(node, ul_timer_t, node);

        /* 如果还没到期，直接返回，因为后面的定时器更不会到期 */
        if (timer->timeout_tick > current_tick)
        {
            return;
        }

        /* 调用回调函数 */
        if (timer->callback != UL_NULL)
        {
            timer->callback(timer->parameter);
        }

        /* 根据定时器类型处理 */
        if (timer->type == ULOS_TIMER_TYPE_PERIODIC)
        {
            /* 周期性定时器，计算下次触发时间 */
            timer->timeout_tick = current_tick + timer->init_tick;
            /* 重新插入到正确位置 */
            ul_timer_insert_sorted(timer);
        }
        else
        {
            /* 单次定时器，停止 */
            ul_list_del_init(&timer->node);
            timer->stat = ULOS_TIMER_STAT_STOPPED;
        }
    }
}

/**
 * @brief 发送定时器命令
 */
static ul_ecode ul_timer_send_cmd(ul_timer_t *timer, ul_timer_cmd_t cmd, ul_tick_t timeout)
{
    if (timer_cmd_queue == UL_NULL)
    {
        return UL_ERROR;
    }

    ul_timer_msg_t msg;
    ul_tick_t current_tick = ulOS_get_tick();

    msg.cmd = cmd;
    msg.timer = timer;
    msg.timeout = timeout + current_tick;
    msg.period = timeout;  /* 用于CHANGE_PERIOD命令 */

    return ul_queue_send(timer_cmd_queue, &msg, sizeof(msg), 0);
}

/* ==================== 定时器API实现 ==================== */

/**
 * @brief 初始化定时器
 */
ul_ecode ul_timer_init(ul_timer_t *timer,
                       const char *name,
                       ul_timer_callback_t callback,
                       void *parameter,
                       ul_tick_t timeout,
                       ul_uint8_t type)
{
    if (timer == UL_NULL)
    {
        return UL_ENULL;
    }

    /* 初始化对象 */
    ul_object_init(name, &timer->parent, UL_OBJECT_CLASS_TIMER);

    /* 初始化定时器参数 */
    timer->timeout_tick = timeout;
    timer->init_tick = timeout;
    timer->type = type;
    timer->stat = ULOS_TIMER_STAT_STOPPED;
    timer->callback = callback;
    timer->parameter = parameter;

    /* 初始化链表节点 */
    ul_list_init(&timer->node);

    return UL_EOK;
}

/**
 * @brief 创建定时器
 */
ul_timer_t* ul_timer_create(const char *name,
                            ul_timer_callback_t callback,
                            void *parameter,
                            ul_tick_t timeout,
                            ul_uint8_t type)
{
    ul_timer_t *timer = ul_malloc(sizeof(ul_timer_t));

    if (timer == UL_NULL)
    {
        return UL_NULL;
    }

    if (ul_timer_init(timer, name, callback, parameter, timeout, type) != UL_EOK)
    {
        ul_free(timer);
        return UL_NULL;
    }

    return timer;
}

/**
 * @brief 删除定时器
 */
ul_ecode ul_timer_delete(ul_timer_t *timer)
{
    if (timer == UL_NULL)
    {
        return UL_ENULL;
    }

    return ul_timer_send_cmd(timer, ULOS_TIMER_CMD_DELETE, 0);
}

/**
 * @brief 启动定时器
 */
ul_ecode ul_timer_start(ul_timer_t *timer)
{
    if (timer == UL_NULL)
    {
        return UL_ENULL;
    }

    return ul_timer_send_cmd(timer, ULOS_TIMER_CMD_START, timer->init_tick);
}

/**
 * @brief 停止定时器
 */
ul_ecode ul_timer_stop(ul_timer_t *timer)
{
    if (timer == UL_NULL)
    {
        return UL_ENULL;
    }

    return ul_timer_send_cmd(timer, ULOS_TIMER_CMD_STOP, 0);
}

/**
 * @brief 重置定时器
 */
ul_ecode ul_timer_reset(ul_timer_t *timer)
{
    if (timer == UL_NULL)
    {
        return UL_ENULL;
    }

    return ul_timer_send_cmd(timer, ULOS_TIMER_CMD_RESET, timer->init_tick);
}

/**
 * @brief 改变定时器周期
 */
ul_ecode ul_timer_change_period(ul_timer_t *timer, ul_tick_t new_period)
{
    if (timer == UL_NULL)
    {
        return UL_ENULL;
    }

    return ul_timer_send_cmd(timer, ULOS_TIMER_CMD_CHANGE_PERIOD, new_period);
}

/**
 * @brief 初始化定时器服务
 */
ul_ecode ul_timer_thread_create(void)
{
    /* 创建命令队列 */
    timer_cmd_queue = ul_queue_create("ostmq", 10, sizeof(ul_timer_msg_t));

    if (timer_cmd_queue == UL_NULL)
    {
        return UL_ERROR;
    }

    /* 创建定时器线程 */
    timer_thread = ul_thread_create("ostm",
                                    timer_thread_entry,
                                    UL_NULL,
                                    ULOS_CONFIG_TIMER_THREAD_STACK_SIZE,/* 栈大小 */
                                    ULOS_CONFIG_TIMER_THREAD_PRIORITY,  /* 优先级 */
                                    1);

    if (timer_thread == UL_NULL)
    {
        ul_queue_delete(timer_cmd_queue);
        timer_cmd_queue = UL_NULL;
        return UL_ERROR;
    }

    /* 启动定时器线程 */
    return ul_thread_startup(timer_thread);
}
