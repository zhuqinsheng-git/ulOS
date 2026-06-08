#include "ulOS_example.h"
#if (ULOS_CONFIG_USE_QUEUE != 0)
#include "ul_thread.h"
#include "ul_ipc.h"
#include "ulos_example.h"
/* 测试用的消息结构体 */
typedef struct {
    char name[16];    /* 字符串 */
    int number;       /* 数字 */
} test_msg_t;

/* 全局队列 */
ul_thread_t *producer_tid;
ul_thread_t *consumer_tid;
ul_queue_t *test_queue;

/* 生产者线程 */
void producer_thread(void *arg)
{
    test_msg_t msg;
    int i = 0;
    ul_ecode err;

    ul_kprintf("Producer thread started\r\n");

    while (1) {
        /* 准备消息 */
        snprintf(msg.name, sizeof(msg.name), "msg%d", i);
        msg.number = i * 100;

        /* 发送消息 */
        err = ul_queue_send_urgent(test_queue, &msg, sizeof(test_msg_t), UL_FALSE);
        ul_enter_critical();
        if (err == UL_EOK) {
            
            ul_kprintf("Producer sent: %s, %d\r\n", msg.name, msg.number);
            
        } else {
            ul_kprintf("Producer send failed (queue full): %s, %d\r\n", msg.name, msg.number);
        }
        ul_exit_critical();
        i++;
        
        /* 延时一段时间 */
        ul_thread_delay(300);  /* 延时1秒 */
    }
}

/* 消费者线程 */
void consumer_thread(void *arg)
{
    test_msg_t recv_msg;
    ul_ecode err;

    ul_kprintf("Consumer thread started\r\n");

    while (1) {
        /* 接收消息 */
        err = ul_queue_receive(test_queue, &recv_msg, sizeof(test_msg_t), ULOS_MAX_DELAY);
        ul_enter_critical();
        if (err == UL_EOK) {
            
            ul_kprintf("Consumer received: %s, %d\r\n", recv_msg.name, recv_msg.number);

        } else {
            ul_kprintf("Consumer receive failed\r\n");
        }
        ul_exit_critical();
        /* 模拟处理时间 */
        ul_thread_delay(500);  /* 延时500ms */
    }
}

/* 队列测试函数 */
void queue_test_start_thread(void *p)
{
    ul_kprintf("=== Multi-thread Queue Test Start ===\r\n");


    test_queue = ul_queue_create("test_queue", 5, sizeof(test_msg_t));
    
    ul_kprintf("Queue created successfully\r\n");

    /* 创建生产者线程 */
    producer_tid = ul_thread_create("producer", producer_thread, NULL, 
                        1024, 1, 1);

    ul_kprintf("Producer thread created\r\n");
    ul_thread_startup(producer_tid);
    
    /* 创建消费者线程 */
    consumer_tid = ul_thread_create("consumer", consumer_thread, NULL,
                        1024, 1, 1);
    ul_kprintf("Consumer thread created\r\n");
    ul_thread_startup(consumer_tid);
    
    /* 主线程等待一段时间 */
    ul_thread_delay(10000);  /* 运行10秒 */

    /* 删除线程 */
    ul_thread_delete(producer_tid);
//    ul_thread_delete(consumer_tid);

    /* 删除队列 */
    //ul_queue_delete(test_queue);

    ul_kprintf("=== Multi-thread Queue Test End ===\r\n");
    
    return;
}

/* 测试函数 */
void example_queue(void)
{
    struct ul_thread *main;
    
    
    /* 创建低优先级任务 */
    main = ul_thread_create("start",
                               queue_test_start_thread,
                               NULL,
                               1024,
                               0,
                               10);
    ul_thread_startup(main);
    
}
#endif /* ULOS_CONFIG_USE_QUEUE != 0 */
