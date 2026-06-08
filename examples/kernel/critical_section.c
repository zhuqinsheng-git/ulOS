#include "ul_thread.h"
#include "ul_mutex.h"
#include "ulos_example.h"
#define USE_CRITICAL_SECTION    2

uint32_t shared_counter = 0;
ul_thread_t *test1;
ul_thread_t *test2;

// 测试线程1
static void test_thread1(void* p)
{
    ul_base_t level;
    ul_mutex_t *mutex= ul_mutex_find("critical_mutex");
    while(1)
    {
#if USE_CRITICAL_SECTION == 1
        ul_enter_critical();
        for(int i = 0;i < 500000;i++)
        {
            shared_counter++;
        }
        ul_exit_critical();
#elif USE_CRITICAL_SECTION == 2
        ul_mutex_lock(mutex, ULOS_MAX_DELAY);
        for(int i = 0;i < 500000;i++)
        {
            shared_counter++;
        }
        ul_mutex_unlock(mutex);
#else
        for(int i = 0;i < 500000;i++)
        {
            shared_counter++;
        }
#endif
        ul_thread_delay(1000);
        return;
    }
}

// 测试线程2
static void test_thread2(void* p)
{
    ul_base_t level;
    ul_mutex_t *mutex= ul_mutex_find("critical_mutex");
    while(1)
    {
#if USE_CRITICAL_SECTION == 1
        ul_enter_critical();
        for(int i = 0;i < 500000;i++)
        {
            shared_counter++;
        }
        ul_exit_critical();
#elif USE_CRITICAL_SECTION == 2
        ul_mutex_lock(mutex, ULOS_MAX_DELAY);
        for(int i = 0;i < 500000;i++)
        {
            shared_counter++;
        }
        ul_mutex_unlock(mutex);
#else
        for(int i = 0;i < 500000;i++)
        {
            shared_counter++;
        }
#endif
        ul_thread_delay(1000);
        return;
    }
}




void example_critical_section(void)
{

    ul_mutex_create("critical_mutex");
    
    test1 = ul_thread_create("test1", test_thread1, NULL, 512, 0, 1);
    test2 = ul_thread_create("test2", test_thread2, NULL, 512, 0, 1);

    
    ul_thread_startup(test1);
    ul_thread_startup(test2);
}
