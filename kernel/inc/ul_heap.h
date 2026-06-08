/**
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * 不与ulOS强绑定，可单独作为模块使用
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-03     zhuqinsheng   the first version
 * 2025-12-08     zhuqinsheng   删除内存屏障
 */
#ifndef UL_HEAP_H
#define UL_HEAP_H

#include "ul_config.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* 堆大小配置 */
#ifndef UL_HEAP_SIZE
#define UL_HEAP_SIZE          ((ul_size_t)(1024 * 1))   /* 堆大小 1KB */
#endif

/**
 * @brief 堆初始化
 */
void ul_heap_init(void);

/**
 * @brief 内存分配
 */
void *ul_malloc(ul_size_t size);

/**
 * @brief 内存释放
 */
void ul_free(void *ptr);

/**
 * @brief 内存重分配
 */
void *ul_realloc(void *ptr, ul_size_t size);

/* 工具函数 */
ul_size_t   ul_heap_get_total_size(void);
ul_size_t   ul_heap_get_used_size(void);
ul_size_t   ul_heap_get_free_size(void);
ul_size_t   ul_heap_get_max_used_size(void);
ul_uint32_t ul_heap_get_alloc_fail_count(void);

/**
 * @brief 获取堆统计信息
 * 
 * - 总体使用率
 * - 每个块的状态
 * - 空闲链表结构
 * - 历史统计信息
 */
//void ul_heap_print(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* UL_HEAP_H */
