#include "ulOS_example.h"
#if (ULOS_CONFIG_USE_EVENT != 0)

#include "ul_thread.h"
#include "ul_ipc.h"

#define THREAD_PRIORITY      2
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     1

#define EVENT_FLAG3 (1 << 3)
#define EVENT_FLAG5 (1 << 5)

/* 事件控制块指针 */
static struct ul_event *event;

/* 线程1入口函数 */
static void thread1_recv_event(void *param)
{
    ul_uint32_t e;

    /* 第一次接收事件，事件3或事件5任意一个可以触发线程1 */
    if (ul_event_recv(event, (EVENT_FLAG3 | EVENT_FLAG5),
                      UL_EVENT_FLAG_OR | UL_EVENT_FLAG_CLEAR,
                      ULOS_MAX_DELAY, &e) == UL_EOK)
    {
        ul_kprintf("thread1: OR recv event 0x%x\r\n", e);
    }

    ul_kprintf("thread1: delay 1s to prepare the second event\r\n");
    ul_thread_delay(1000);

    /* 第二次接收事件，事件3和事件5均发生时才可以触发线程1 */
    if (ul_event_recv(event, (EVENT_FLAG3 | EVENT_FLAG5),
                      UL_EVENT_FLAG_AND | UL_EVENT_FLAG_CLEAR,
                      ULOS_MAX_DELAY, &e) == UL_EOK)
    {
        ul_kprintf("thread1: AND recv event 0x%x\r\n", e);
    }
    ul_kprintf("thread1 leave.\r\n");
    
    ul_event_delete(event);
    
    return;
}

/* 线程2入口 */
static void thread2_send_event(void *param)
{
    ul_kprintf("thread2: send event3\r\n");
    ul_event_send(event, EVENT_FLAG3);
    ul_thread_delay(200);

    ul_kprintf("thread2: send event5\r\n");
    ul_event_send(event, EVENT_FLAG5);
    ul_thread_delay(200);

    ul_kprintf("thread2: send event3\r\n");
    ul_event_send(event, EVENT_FLAG3);
    ul_kprintf("thread2 leave.\r\n");

    /* 线程退出时删除自己 */  
    return;
}

int example_event(void)
{
    struct ul_thread *thread1, *thread2;

    /* 创建事件对象 */
    event = ul_event_create("event");
    if (event == NULL)
    {
        ul_kprintf("create event failed.\r\n");
        return -1;
    }

    /* 创建线程1 */
    thread1 = ul_thread_create("eventt1",
                             thread1_recv_event,
                             NULL,
                             THREAD_STACK_SIZE,
                             THREAD_PRIORITY - 1,
                             THREAD_TIMESLICE);
    if (thread1 == NULL)
    {
        ul_kprintf("create thread1 failed.\r\n");
        ul_event_delete(event);
        return -1;
    }
    ul_thread_startup(thread1);
    
    /* 创建线程2 */
    thread2 = ul_thread_create("eventt2",
                             thread2_send_event,
                             NULL,
                             THREAD_STACK_SIZE,
                             THREAD_PRIORITY,
                             THREAD_TIMESLICE);
    if (thread2 == NULL)
    {
        ul_kprintf("create thread2 failed.\r\n");
        ul_thread_delete(thread1);
        ul_event_delete(event);
        return -1;
    }
    ul_thread_startup(thread2);
    return 0;
}
#endif /* ULOS_CONFIG_USE_EVENT != 0 */
