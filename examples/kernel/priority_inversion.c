#include "ulos_example.h"
#include "ul_thread.h"
#include "ul_mutex.h"

/* 定义三个任务，优先级从高到低 */
#define HIGH_PRIORITY     0
#define MEDIUM_PRIORITY   1
#define LOW_PRIORITY      2

/* 共享资源计数器 */
static ul_uint32_t shared_counter = 0;

/* 互斥锁 */
static ul_mutex_t test_mutex;

/* 高优先级任务 */
void high_priority_task(void *param)
{
    /* 让低优先级任务先运行并获取互斥锁 */
    ul_thread_delay(100);
    while (1) {
        /* 尝试获取互斥锁 */
        if (ul_mutex_lock(&test_mutex, ULOS_MAX_DELAY) == UL_EOK) {
            /* 访问共享资源 */
            shared_counter++;
            ul_kprintf("High priority task: counter = %d\r\n", shared_counter);
            
            /* 模拟处理时间 */
            ul_thread_delay(10);
            
            /* 释放互斥锁 */
            ul_mutex_unlock(&test_mutex);
        }
        
        /* 任务延时，让其他任务有机会运行 */
        ul_thread_delay(100);
    }
}

/* 中优先级任务 */
void medium_priority_task(void *param)
{
    /* 让低优先级任务先运行并获取互斥锁 */
    ul_thread_delay(100);
    while (1) {
        /* 模拟CPU密集型任务 */
        for (volatile ul_uint32_t i = 0; i < 1000000; i++);
        
        ul_kprintf("Medium priority task running\r\n");
        
        /* 短暂延时 */
        ul_thread_delay(50);
    }
}

/* 低优先级任务 */
void low_priority_task(void *param)
{
    while (1) {
        /* 获取互斥锁 */
        if (ul_mutex_lock(&test_mutex, ULOS_MAX_DELAY) == UL_EOK) {
            ul_kprintf("Low priority task got mutex\r\n");
            
            /* 模拟长时间占用共享资源 */
            ul_thread_delay(500);
            
            /* 访问共享资源 */
            shared_counter++;
            ul_kprintf("Low priority task: counter = %d\r\n", shared_counter);
            
            /* 释放互斥锁 */
            ul_mutex_unlock(&test_mutex);
        }
        
        /* 任务延时 */
        ul_thread_delay(100);
    }
}

/* 测试函数 */
void example_priority_inversion(void)
{
    struct ul_thread *high_task, *medium_task, *low_task;
    
    /* 初始化互斥锁 */
    ul_mutex_init(&test_mutex, "te");
    
    /* 创建低优先级任务 */
    low_task = ul_thread_create("low",
                               low_priority_task,
                               NULL,
                               512,
                               LOW_PRIORITY,
                               10);
    ul_thread_startup(low_task);
    
    /* 让低优先级任务先运行并获取互斥锁 */
    //ul_thread_delay(100);
    
    /* 创建中优先级任务 */
    medium_task = ul_thread_create("medium",
                                  medium_priority_task,
                                  NULL,
                                  512,
                                  MEDIUM_PRIORITY,
                                  10);
    ul_thread_startup(medium_task);
    
    /* 创建高优先级任务 */
    high_task = ul_thread_create("high",
                                high_priority_task,
                                NULL,
                                512,
                                HIGH_PRIORITY,
                                10);
    ul_thread_startup(high_task);
}
