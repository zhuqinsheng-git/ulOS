/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-12-17     zhuqinsheng   the first version
 */
#ifndef _UL_TYPES_H
#define _UL_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Basic Types */
typedef float               ul_fp32_t;
typedef double              ul_fp64_t;
typedef int8_t              ul_int8_t;
typedef uint8_t             ul_uint8_t;
typedef int16_t             ul_int16_t;
typedef uint16_t            ul_uint16_t;
typedef int32_t             ul_int32_t;
typedef uint32_t            ul_uint32_t;
typedef size_t              ul_size_t;
typedef unsigned long long  ul_tick_t;
typedef uint32_t            ul_base_t;
typedef bool                ul_bool_t;

/* Constants */
#define UL_TRUE             true
#define UL_FALSE            false
#define UL_NULL             (0)
#define UL_ALIGN_SIZE       4
#define UL_OBJECT_NAME_MAX_LENGTH   (24)

/* Macros */
#define ul_inline           static __inline
#define ul_weak             __attribute__((weak))
#define UL_ALIGN_DOWN(size,align)   ((size)&~((align)-1))
#define UL_ALIGN_4(size)    ((size)&~((4)-1))

/* Error Codes */
typedef enum {
    /* Success */
    UL_EOK = 0,             /* 成功 */

    /* General Errors */
    UL_ERROR = -1,          /* 通用错误 */
    UL_ENULL = -2,          /* 空指针错误 */
    UL_EINVAL = -3,         /* 无效参数 */
    UL_ENOMEM = -4,         /* 内存不足 */
    UL_ENOENT = -5,         /* 对象不存在 */
    UL_EPERM = -6,          /* 操作不被允许 */
    UL_EBUSY = -7,          /* 资源忙 */
    UL_EEXIST = -8,         /* 对象已存在 */

    /* Timeout Errors */
    UL_ETIMEOUT = -9,       /* 操作超时 */

    /* IPC Errors */
    UL_EFULL = -10,         /* 队列/缓冲区满 */
    UL_EEMPTY = -11,        /* 队列/缓冲区空 */

    /* Thread Errors */
    UL_ETHREAD = -12,       /* 线程错误 */
} ul_ecode;




#ifdef __cplusplus
}
#endif

#endif /* UL_TYPES_H */
