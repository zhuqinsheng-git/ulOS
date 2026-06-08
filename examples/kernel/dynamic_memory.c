#include "ul_thread.h"
#include "ulos_example.h"
static ul_thread_t *thread1;

static void thread1_entry(void *parameter)
{ 
    int i;
    char *ptr = UL_NULL; /* 内存块的指针 */
    
    ul_kprintf("ulOS heap total size :%d byte\r\n", ul_heap_get_total_size());
    
    for (i = 0; ; i++)
    {
        /* 每次分配 (1 << i) 大小字节数的内存空间 */
        ptr = ul_malloc(1 << i);

        /* 如果分配成功 */
        if (ptr != UL_NULL)
        {
            ul_kprintf("get memory :%d byte\r\n", (1 << i));
            /* 释放内存块 */
            ul_free(ptr);
            ul_kprintf("free memory :%d byte\r\n", (1 << i));
            ptr = UL_NULL;
        }
        else
        {
            ul_kprintf("try to get %d byte memory failed!\r\n", (1 << i));
            ul_kprintf("ulOS heap free size :%d byte\r\n", ul_heap_get_free_size());
            return;
        }
    }
}

/* 测试函数 */
void example_dynamic_memmory(void)
{
    thread1 = ul_thread_create("ul_malloc_t1",
                               thread1_entry,
                               NULL,
                               512,
                               1,
                               1);
    ul_thread_startup(thread1);
}
