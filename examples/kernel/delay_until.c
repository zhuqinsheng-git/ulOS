#include "ul_thread.h"
#include "ulos_example.h"
static ul_thread_t *thread1;
static ul_thread_t *thread2;

static void thread1_entry(void *parameter)
{ 
    volatile ul_uint32_t i = 0;
    uint8_t j = 0;
    
    while(j < 10)
    {
        ul_kprintf("[1]delay_thread running...ulOS tick:%d\r\n", (int)ulOS_get_tick());
        
        for(i = 0; i < 100000; i++);    // 模拟运行耗时
        
        ul_thread_delay(500);  
        
        j++;
    }
    
    ul_kprintf("[1]delay_thread delete...\r\n");
    return;
}

static void thread2_entry(void *parameter)
{
    volatile ul_uint32_t i = 0;
    uint8_t j = 0;
    
    ul_tick_t last_wake_time = ulOS_get_tick();
    
    while(j < 10)
    {
        ul_kprintf("[2]delay_until_thread running...ulOS tick:%d\r\n", (int)ulOS_get_tick());
        
        for(i = 0; i < 100000; i++);    // 模拟运行耗时
        
        ul_thread_delay_until(&last_wake_time, 500);
        
        j++;
    }   
    
    ul_kprintf("[2]delay_until_thread delete...\r\n");
    return;
}

/* 测试函数 */
void example_delay_until(void)
{
    thread1 = ul_thread_create("du_t1",
                               thread1_entry,
                               NULL,
                               512,
                               1,
                               1);
    ul_thread_startup(thread1);
    
    thread2 = ul_thread_create("du_t2",
                               thread2_entry,
                               NULL,
                               512,
                               0,
                               1);
    ul_thread_startup(thread2);
    
}
