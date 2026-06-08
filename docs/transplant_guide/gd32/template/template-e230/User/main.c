#include "gd32e23x.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"

#include "gd32e23x_gpio.h"

#include "ulOS_thread.h"

#define  BSP_TIMER_RCU        RCU_TIMER5  // 定时器时钟
#define  BSP_TIMER            TIMER5   // 定时器
#define  BSP_TIMER_IRQ        TIMER5_IRQn // 定时器中断

#define  BSP_TIMER_IRQHandler TIMER5_IRQHandler// 定时器中断服务函数

#define  BSP_LED1_RCU     RCU_GPIOC
#define  BSP_LED1_PORT    GPIOC
#define  BSP_LED1_PIN     GPIO_PIN_13

#define  BSP_LED2_RCU     RCU_GPIOA
#define  BSP_LED2_PORT    GPIOA
#define  BSP_LED2_PIN     GPIO_PIN_1

void thread1_entry(void *p)
{
    while(1)
    {
        gpio_bit_toggle(BSP_LED1_PORT,BSP_LED1_PIN); // 翻转led
        ul_thread_delay(100);
    }
}

void thread2_entry(void *p)
{
    while(1)
    {
        gpio_bit_toggle(BSP_LED2_PORT,BSP_LED2_PIN); // 翻转led
        ul_thread_delay(500);
    }
}

void start_thread_entry(void *p)
{
    ul_thread_t *tid1 = ul_thread_create("t1", thread1_entry, UL_NULL, 512, 0, 1);
    ul_thread_t *tid2 = ul_thread_create("t2", thread2_entry, UL_NULL, 512, 0, 1);
    
    ul_thread_startup(tid1);
    ul_thread_startup(tid2);
    
    while(1)
    {
        ul_thread_delay(10);
    }
}

int main(void)
{
    systick_config();
    
    rcu_periph_clock_enable(BSP_LED1_RCU);
    gpio_mode_set(BSP_LED1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BSP_LED1_PIN);
    gpio_output_options_set(BSP_LED1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, BSP_LED1_PIN);
    
    rcu_periph_clock_enable(BSP_LED2_RCU);
    gpio_mode_set(BSP_LED2_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BSP_LED2_PIN);
    gpio_output_options_set(BSP_LED2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, BSP_LED2_PIN);
    
    /* 开启时钟 */
    rcu_periph_clock_enable(BSP_TIMER_RCU); // 开启定时器时钟
    timer_deinit(BSP_TIMER); // 复位定时器
    
    timer_parameter_struct timer_initpara; // 定义定时器结构体
    /* 配置定时器参数 */
    timer_initpara.prescaler = 2 -1; //  时钟预分频值 0-65535  psc_clk = CK_TIMER / pre
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE; // 边缘对齐（没用到）
    timer_initpara.counterdirection = TIMER_COUNTER_UP; // 向上计数
    timer_initpara.period = 36000  - 1; // 周期
    /* 在输入捕获的时候使用  数字滤波器使用的采样频率之间的分频比例 */
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1; // 分频因子
    /* 只有高级定时器才有 配置为x，就重复x+1次进入中断 */
    timer_initpara.repetitioncounter = 0; // 重复计数器 0-255
    timer_init(BSP_TIMER,&timer_initpara); // 初始化定时器
    /* 配置中断优先级 */
    nvic_irq_enable(BSP_TIMER_IRQ, 1);    // 设置中断优先级
    /* 使能中断 */
    timer_interrupt_enable(BSP_TIMER,TIMER_INT_UP); // 使能更新事件中断
    /* 使能定时器 */
    timer_enable(BSP_TIMER);
    
    
    // 配置PC13引脚转化代码为
    
    ul_kernel_init();
    
    ul_thread_startup(ul_thread_create("start",start_thread_entry,UL_NULL,512,1,1));
    
    ul_scheduler_start();
    
    while(1)
    {
        
    }   
}

// 1ms定时器
void BSP_TIMER_IRQHandler(void){
    if(timer_interrupt_flag_get(BSP_TIMER,TIMER_INT_FLAG_UP) == SET)
    {
        timer_interrupt_flag_clear(BSP_TIMER,TIMER_INT_FLAG_UP); // 清除中断标志位

        ul_tick_increase();
    }
}