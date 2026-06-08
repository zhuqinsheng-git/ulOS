/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-01     zhuqinsheng   the first version, merge ul_list
 */
#ifndef _UL_OBJECT_H
#define _UL_OBJECT_H

#include "ul_config.h"
#include "ul_heap.h"
#include "ul_list.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */



/* ======================================================== */
/* ----------------------- OBJECT ------------------------- */
/* ======================================================== */
#define UL_OBJECT_FLAG_STATIC      0x01
#define UL_OBJECT_FLAG_DYNAMIC     0x02

typedef enum
{
    UL_OBJECT_CLASS_UNKNOWN,
    UL_OBJECT_CLASS_THREAD = 0x01,
#if (ULOS_CONFIG_USE_MUTEX != 0)
    UL_OBJECT_CLASS_MUTEX,
#endif
#if (ULOS_CONFIG_USE_QUEUE != 0)
    UL_OBJECT_CLASS_QUEUE,
#endif
#if (ULOS_CONFIG_USE_SEMAPHORE != 0)
    UL_OBJECT_CLASS_SEMAPHORE,
#endif
#if (ULOS_CONFIG_USE_EVENT != 0)
    UL_OBJECT_CLASS_EVENT,
#endif
#if (ULOS_CONFIG_USE_TIMER != 0)
    UL_OBJECT_CLASS_TIMER,
#endif
    UL_OBJECT_CLASS_SWTIMER,
#if (ULOS_CONFIG_USE_TOPIC != 0)
    UL_OBJECT_CLASS_TOPIC,
#endif
    UL_OBJECT_CLASS_PIN,
    UL_OBJECT_CLASS_MAX
} ul_object_class_type;
    
// 对象基类
typedef struct ul_object 
{
    char                        name[UL_OBJECT_NAME_MAX_LENGTH];
    ul_uint8_t                  flag;
    ul_object_class_type        type;
    ul_list_t                   node;
} ul_object_t;

// 对象管理接口
ul_ecode ul_object_init(const char* name, ul_object_t* object, ul_object_class_type type);
ul_ecode ul_object_unregister(ul_object_t* object);
ul_object_t* ul_object_find(const char* name, ul_object_class_type type);

typedef int (*ul_output_func_t)(const char *format, ...);
ul_ecode ul_object_list(ul_output_func_t kprtinf);


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif

