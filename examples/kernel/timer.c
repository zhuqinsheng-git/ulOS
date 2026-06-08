#include "ul_timer.h"
#include "ulos_example.h"
#if ( ULOS_CONFIG_USE_TIMER == 1 )

/* 定时器的控制块 */
ul_timer_t *timer1;
ul_timer_t *timer2;
static int cnt = 0;

/* 定时器1超时函数 */
static void timeout1(void *parameter)
{
    ul_kprintf("periodic timer is timeout %d\n", cnt);

    /* 运行第10次，停止周期定时器 */
    if (cnt++ >= 9)
    {
        ul_timer_stop(timer1);
        ul_kprintf("periodic timer was stopped! \n");
    }
}

/* 定时器2超时函数 */
static void timeout2(void *parameter)
{
    ul_kprintf("one shot timer is timeout\n");
}

int example_timer(void)
{
    /* 创建定时器1  周期定时器 */
    timer1 = ul_timer_create("timer1", timeout1,
                             UL_NULL, 30,
                             ULOS_TIMER_TYPE_PERIODIC);

    /* 启动定时器1 */
    if (timer1 != UL_NULL) ul_timer_start(timer1);

    /* 创建定时器2 单次定时器 */
    timer2 = ul_timer_create("timer2", timeout2,
                             UL_NULL,  10,
                             ULOS_TIMER_TYPE_ONESHOT);

    /* 启动定时器2 */
    if (timer2 != UL_NULL) ul_timer_start(timer2);
    return 0;
}
#endif /* ULOS_CONFIG_USE_TIMER == 1 */
