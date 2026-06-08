/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-03     zhuqinsheng   the first version
 * 2025-12-08     zhuqinsheng   删除内存屏障
 */
#include "ul_heap.h"

/* 内存对齐配置 */
#define HEAP_ALIGNMENT          ( 8 )

#define MEM_ALIGN_SIZE(size)    (((size) + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1))

#define MEM_BLOCK_HEADER_SIZE   MEM_ALIGN_SIZE(sizeof(ul_memblock_t))
    
#define IS_VALID_BLOCK(block) \
    ((uint8_t *)(block) >= g_heap_manager.start_addr && \
     (uint8_t *)(block) < g_heap_manager.end_addr && \
     ((ul_memblock_t *)(block))->size >= MEM_BLOCK_HEADER_SIZE && \
     ((ul_memblock_t *)(block))->size <= UL_HEAP_SIZE)
     
#define CHECK_HEAP_IS_INITIALIZED()           \
        do{if (g_heap_manager.end_addr == UL_NULL)\
                    _heap_init();}while(0)
    
/* 堆 */
static uint8_t g_heap[UL_HEAP_SIZE];

/* 内存块头部结构 */
typedef struct memblock
{
    struct memblock *next;      /* 下一个空闲块 */
    ul_size_t       size;       /* 块大小（包含头部） */
} ul_memblock_t;

/* 堆管理结构 */
typedef struct ul_heap_manager
{
    ul_uint8_t      *start_addr;     /* 堆起始地址 */
    ul_uint8_t      *end_addr;       /* 堆结束地址 */
    ul_size_t       total_size;      /* 总大小 */
    ul_size_t       used_size;       /* 已使用大小 */
    ul_size_t       max_used_size;   /* 最大使用大小 */
    ul_memblock_t   *free_list;      /* 空闲链表 */

    /* 统计信息 */
    ul_uint32_t     alloc_count;     /* 分配次数 */
    ul_uint32_t     free_count;      /* 释放次数 */
    ul_uint32_t     alloc_fail_count;/* 分配失败次数 */
} ul_heap_manager_t;
static struct ul_heap_manager g_heap_manager;

/**
 * @brief 堆初始化
 *
 * 初始化堆管理结构，创建初始空闲块：
 * - 设置堆边界
 * - 初始化统计信息
 * - 创建初始空闲块
 *
 * @note 无需用户调用
 */
static void _heap_init(void)
{
    uint8_t *heap_start = (uint8_t*)g_heap;

    /* 初始化堆结构 */
    g_heap_manager.start_addr = heap_start;
    g_heap_manager.total_size = UL_HEAP_SIZE;
    g_heap_manager.end_addr = heap_start + UL_HEAP_SIZE;
    g_heap_manager.used_size = 0;
    g_heap_manager.max_used_size = 0;
    g_heap_manager.alloc_count = 0;
    g_heap_manager.free_count = 0;
    g_heap_manager.alloc_fail_count = 0;

    /* 初始化第一个大空闲块 */
    ul_memblock_t *first_block = (ul_memblock_t *)heap_start;
    first_block->size = UL_HEAP_SIZE;
    first_block->next = UL_NULL;

    g_heap_manager.free_list = first_block;
}

/**
 * @brief 从堆分配内存
 * @param size 需要分配的内存大小（字节）
 * @return 成功返回分配的内存指针，失败返回NULL
 */
void *ul_malloc(ul_size_t size)
{
    CHECK_HEAP_IS_INITIALIZED();
    if (size == 0 || (size + MEM_BLOCK_HEADER_SIZE ) > UL_HEAP_SIZE)
    {
        return UL_NULL;
    }

    /* 计算需要分配的总大小（包括头部和对齐） */
    ul_size_t total_size = MEM_BLOCK_HEADER_SIZE + MEM_ALIGN_SIZE(size);

    ul_memblock_t *prev_block = UL_NULL;
    ul_memblock_t *curr_block = g_heap_manager.free_list;
    ul_memblock_t *best_prev = UL_NULL;
    ul_memblock_t *best_block = UL_NULL;

    /* 使用最佳适配算法查找合适的空闲块 */
    while (curr_block != UL_NULL)
    {
        if (curr_block->size >= total_size)
        {
            /* 找到合适的块，检查是否是最佳选择 */
            if (best_block == UL_NULL || curr_block->size < best_block->size)
            {
                best_block = curr_block;
                best_prev = prev_block;
            }
        }

        prev_block = curr_block;
        curr_block = curr_block->next;
    }

    /* 没有找到合适的块 */
    if (best_block == UL_NULL)
    {
        g_heap_manager.alloc_fail_count++;
        return UL_NULL;
    }

    /* 计算剩余空间是否足够形成新的空闲块 */
    ul_size_t remaining_size = best_block->size - total_size;

    if (remaining_size > MEM_BLOCK_HEADER_SIZE)
    {
        /* 分割块：创建新的空闲块 */
        ul_memblock_t *new_free_block = (ul_memblock_t *)((uint8_t *)best_block + total_size);
        new_free_block->size = remaining_size;
        new_free_block->next = best_block->next;

        /* 更新链表 */
        if (best_prev != UL_NULL)
        {
            /* 场景：最佳块在链表中间
               free_list → [Block1] → [Block2: size=200] → [Block3] → UL_NULL
               best_prev    best_block */
            best_prev->next = new_free_block;
        }
        else
        {
            /* 场景：最佳块在链表头部
             free_list → [Block1: size=200] → [Block2] → UL_NULL
                           best_block
                           best_prev = UL_NULL */
            g_heap_manager.free_list = new_free_block;
        }

        /* 更新分配块的大小 */
        best_block->size = total_size;
    }
    else
    {
        /* 不分割，整个块都分配出去 */
        if (best_prev != UL_NULL)
        {
            best_prev->next = best_block->next;
        }
        else
        {
            g_heap_manager.free_list = best_block->next;
        }
    }

    /* 更新统计信息 */
    g_heap_manager.used_size += best_block->size;
    g_heap_manager.alloc_count++;

    if (g_heap_manager.used_size > g_heap_manager.max_used_size)
    {
        g_heap_manager.max_used_size = g_heap_manager.used_size;
    }

    /* 返回用户可用内存地址（跳过头部） */
    return (uint8_t *)best_block + MEM_BLOCK_HEADER_SIZE;
}

/**
 * @brief 释放内存到堆
 * @param ptr 要释放的内存指针
 */
void ul_free(void *ptr)
{
    CHECK_HEAP_IS_INITIALIZED();
    if (ptr == UL_NULL)
    {
        return;
    }

    /* 获取块头部 */
    ul_memblock_t *free_block = (ul_memblock_t *)((uint8_t *)ptr - MEM_BLOCK_HEADER_SIZE);

    if (!IS_VALID_BLOCK(free_block))
    {
        return;
    }
    
    ul_size_t free_size = free_block->size;

    ul_memblock_t *curr_block = g_heap_manager.free_list;
    ul_memblock_t *prev_block = UL_NULL;

    /* 找到插入位置（按地址排序） */
    // 为了找到要释放的内存的最近的前后空闲内存（只是最近的，不代表相邻的就是空闲内存）
    //    内存布局：
    //+-------+------+-------+------+
    //| Free1 | Used | Free2 | Used |
    //+-------+------+-------+------+
    //   ↑      ↑      ↑
    //  prev   要释放   curr
    while (curr_block != UL_NULL && curr_block < free_block)
    {
        prev_block = curr_block;
        curr_block = curr_block->next;
    }

    /* 尝试向前合并 */
    if (prev_block != UL_NULL)
    {
        /* 1.找到前面最近的空闲块的下一个块 */
        uint8_t *prev_end = (uint8_t *)prev_block + prev_block->size;

        /* 2.判断这个块是不是空闲块 */
        if (prev_end == (uint8_t *)free_block)
        {
            /* 3.是：合并前一个块 */
            prev_block->size += free_block->size;
            free_block = prev_block;
        }
        else
        {
            /* 4.不是：不能向前合并，插入到空闲链表中 */
            prev_block->next = free_block;
            free_block->next = curr_block;
        }
    }
    else
    {
        /* 已用块是第一个块，无法向前合并，只能插入到链表头部 */
        free_block->next = g_heap_manager.free_list;
        g_heap_manager.free_list = free_block;
    }

    /* 尝试向后合并 */
    if (curr_block != UL_NULL)
    {
        /* 1.找到下一个块 */
        uint8_t *free_end = (uint8_t *)free_block + free_block->size;

        /* 2.是空闲块 */
        if (free_end == (uint8_t *)curr_block)
        {
            /* 合并后一个块 */
            free_block->size += curr_block->size;
            free_block->next = curr_block->next;
        }
    }

    /* 更新统计信息 */
    g_heap_manager.used_size -= free_size;
    g_heap_manager.free_count++;
}

/**
 * @brief 重新分配内存大小
 *
 * 重新分配内存大小：
 * 1. 如果ptr为NULL，等同于malloc
 * 2. 如果size为0，等同于free
 * 3. 如果新大小小于等于原大小，直接返回原指针
 * 4. 否则分配新内存并复制数据
 *
 * @param ptr 原内存指针
 * @param size 新的内存大小
 * @return 成功返回新内存指针，失败返回NULL
 * @note 原内存指针在成功重分配后会失效
 */
void *ul_realloc(void *ptr, ul_size_t size)
{
    CHECK_HEAP_IS_INITIALIZED();
    if (ptr == UL_NULL)
    {
        return ul_malloc(size);
    }

    if (size == 0)
    {
        ul_free(ptr);
        return UL_NULL;
    }

    /* 获取原块信息 */
    ul_memblock_t *old_block = (ul_memblock_t *)((uint8_t *)ptr - MEM_BLOCK_HEADER_SIZE);
    ul_size_t old_size = old_block->size - MEM_BLOCK_HEADER_SIZE;

    /* 如果新大小小于等于原大小，直接返回原指针 */
    if (size <= old_size)
    {
        return ptr;
    }

    /* 分配新内存 */
    void *new_ptr = ul_malloc(size);

    if (new_ptr != UL_NULL)
    {
        /* 拷贝数据 */
        memcpy(new_ptr, ptr, old_size);
        /* 释放原内存 */
        ul_free(ptr);
    }

    return new_ptr;
}

/**
 * @brief 获取堆总大小
 *
 * @return 堆总大小（字节）
 */
ul_size_t ul_heap_get_total_size(void)
{
    return g_heap_manager.total_size;
}

/**
 * @brief 获取已使用大小
 *
 * @return 当前已使用的内存大小（字节）
 */
ul_size_t ul_heap_get_used_size(void)
{
    return g_heap_manager.used_size;
}

/**
 * @brief 获取空闲大小
 *
 * @return 当前空闲的内存大小（字节）
 */
ul_size_t ul_heap_get_free_size(void)
{
    return g_heap_manager.total_size - g_heap_manager.used_size;
}

/**
 * @brief 获取最大使用大小
 *
 * @return 历史最大使用的内存大小（字节）
 */
ul_size_t ul_heap_get_max_used_size(void)
{
    return g_heap_manager.max_used_size;
}

/**
 * @brief 获取分配失败次数
 */
ul_uint32_t ul_heap_get_alloc_fail_count(void)
{
    return g_heap_manager.alloc_fail_count;
}

/**
 * @brief 打印堆状态信息
 *
 * 打印详细的堆信息：
 * - 总体使用情况（包含百分比）
 * - 所有内存块的状态（地址、大小、百分比）
 * - 空闲链表结构
 * - 统计信息（分配次数、释放次数、失败次数）
 */
//void ul_heap_print(void)
//{
//    CHECK_HEAP_IS_INITIALIZED();

//    ul_kprintf("\r\n=== Heap Status ===\r\n");
//    ul_size_t total = g_heap_manager.total_size;
//    ul_size_t used = g_heap_manager.used_size;
//    ul_size_t free = total - used;
//    ul_size_t max_used = g_heap_manager.max_used_size;

//    ul_kprintf("Total: %zu bytes\r\n", total);
//    ul_kprintf("Used: %zu bytes (%.1f%%)\r\n", used, (float)used * 100 / total);
//    ul_kprintf("Free: %zu bytes (%.1f%%)\r\n", free, (float)free * 100 / total);
//    ul_kprintf("Max Used: %zu bytes (%.1f%%)\r\n", max_used, (float)max_used * 100 / total);

//    ul_kprintf("\r\n============= Heap Blocks =============\r\n");
//    ul_kprintf("Address\t\tSize\t\tPercent\tStatus\r\n");
//    ul_kprintf("--------\t----\t\t-------\t------\r\n");

//    uint8_t *current = g_heap_manager.start_addr;
//    uint8_t *end = g_heap_manager.end_addr;
//    ul_memblock_t *free_block = g_heap_manager.free_list;

//    while (current < end)
//    {
//        ul_memblock_t *block = (ul_memblock_t *)current;

//        // 检查当前块是否在空闲链表中
//        int is_free = 0;
//        ul_memblock_t *temp = free_block;

//        while (temp != UL_NULL)
//        {
//            if (temp == block)
//            {
//                is_free = 1;
//                break;
//            }

//            temp = temp->next;
//        }

//        // 打印块信息
//        ul_kprintf("0x%08X\t%zu\t\t%.1f%%\t%s\r\n",
//               (uint32_t)current,
//               block->size,
//               (float)block->size * 100 / total,
//               is_free ? "FREE" : "USED");

//        // 移动到下一个块
//        current += block->size;
//    }

//    ul_kprintf("\r\n=============== Free Blocks List ===============\r\n");
//    ul_kprintf("Address\t\tSize\t\tPercent\tNext\r\n");
//    ul_kprintf("--------\t----\t\t-------\t----\r\n");

//    free_block = g_heap_manager.free_list;

//    while (free_block != UL_NULL)
//    {
//        ul_kprintf("0x%08X\t%zu\t\t%.1f%%\t0x%08X\r\n",
//               (uint32_t)free_block,
//               free_block->size,
//               (float)free_block->size * 100 / total,
//               free_block->next ? (uint32_t)free_block->next : 0);
//        free_block = free_block->next;
//    }

//    ul_kprintf("\r\n=== Statistics ===\r\n");
//    ul_kprintf("Alloc: %u, Free: %u, Fail: %u\r\n",
//           g_heap_manager.alloc_count,
//           g_heap_manager.free_count,
//           g_heap_manager.alloc_fail_count);
//}
