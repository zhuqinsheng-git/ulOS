#include "ulOS_example.h"
#if (ULOS_CONFIG_USE_SEMAPHORE != 0)

#include "ul_thread.h"
#include "ul_ipc.h"
/* 指向信号量的指针 */
ul_sem_t *dynamic_sem = UL_NULL;

ul_thread_t *sem_thread1;
ul_thread_t *sem_thread2;

static void _thread1_entry(void *parameter)
{
    static ul_uint8_t count = 0;
  
    while(1)
    {
        if(count <= 100)
        {
            count++;           
        }
        else
            return; 
        
        /* count每计数10次，就释放一次信号量 */
         if(0 == (count % 10))
        {
            ul_sem_give(dynamic_sem);   
            ul_enter_critical();
            ul_kprintf("t1 release a dynamic semaphore.\r\n" ); 
            ul_exit_critical();
        }
    }
}



static void _thread2_entry(void *parameter)
{
    static ul_ecode result;
    static ul_uint8_t number = 0;
    while(1)
    {
        /* 永久方式等待信号量，获取到信号量，则执行number自加的操作 */
        result = ul_sem_take(dynamic_sem, ULOS_MAX_DELAY);
        ul_enter_critical();
        if (result != UL_EOK)
        {        
            ul_kprintf("t2 take a dynamic semaphore, failed.\r\n");
            ul_sem_delete(dynamic_sem);
            return;
        }
        else
        {      
            number++;             
            ul_kprintf("t2 take a dynamic semaphore. number = %d\r\n" ,number);                        
        }
        ul_exit_critical();
    }   
}

/* 测试函数 */
void example_semaphore(void)
{
    
    /* 创建一个动态信号量，初始值是0 */
    dynamic_sem = ul_sem_create("dsem", 0);
    if (dynamic_sem == UL_NULL)
    {
        ul_kprintf("create dynamic semaphore failed.\r\n");
        return;
    }
    else
    {
        ul_kprintf("create done. dynamic semaphore value = 0.\r\n");
    }

    sem_thread1 = ul_thread_create("sem_t1",
                               _thread1_entry,
                               NULL,
                               1024,
                               1,
                               1);
    ul_thread_startup(sem_thread1);
    
    sem_thread2 = ul_thread_create("sem_t2",
                               _thread2_entry,
                               NULL,
                               1024,
                               0,
                               1);
    ul_thread_startup(sem_thread2);
    
}
#endif /* ULOS_CONFIG_USE_SEMAPHORE != 0 */
