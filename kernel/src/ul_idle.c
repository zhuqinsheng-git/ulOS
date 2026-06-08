/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-7      zhuqinsheng   the first version
 */
 
#include "ul_thread.h"

static struct ul_thread *idle_thread_handle;
ul_list_t ul_defunct_thread_list = UL_LIST_HEAD_INIT(ul_defunct_thread_list);

static void idle_thread_entry(void *p);

void ul_idle_thread_create(void)
{
    idle_thread_handle =
    ul_thread_create("idle",
                     idle_thread_entry,
                     NULL,
                     512,
                     ULOS_CONFIG_MAX_PRIORITY - 1,
                     1);
    ul_thread_startup(idle_thread_handle);
}

static void idle_thread_entry(void *p)
{
//    static ul_tick_t last_tick = 0;
//    static ul_tick_t last_sec = 0;
//    static ul_tick_t idle_count = 0;
//    static ul_uint8_t cpu_usage;

    while (1)
    {
//        {
//            // CPU使用率统计
//            if (ulOS_tick - last_tick >= 1)
//            {
//                idle_count++;
//                last_tick = ulOS_tick;
//            }

//            if (ulOS_tick - last_sec >= 1000)
//            {
//                cpu_usage = 100 - (idle_count * 100 / 1000);

//                idle_count = 0;
//                last_sec = ulOS_tick;
//            }
//        }
#if ( ULOS_CONFIG_USE_IDLEHOOK == 1 )
{
    extern void ulOS_idle_hook(void);
    ulOS_idle_hook();
}
#endif
        if (ul_defunct_thread_list.next != &ul_defunct_thread_list)
        {
            struct ul_thread *thread = ul_list_entry(ul_defunct_thread_list.next,
                                        struct ul_thread,
                                        tlist);
            ul_list_del_init(&thread->tlist);
            ul_object_unregister(&thread->parent);
            ul_free(thread->stack_start);
            ul_free(thread);
        }
        
    }
}
