#include "ul_config.h"

struct stack_frame
{
    /* r4 ~ r11 register */
    ul_uint32_t r4;
    ul_uint32_t r5;
    ul_uint32_t r6;
    ul_uint32_t r7;
    ul_uint32_t r8;
    ul_uint32_t r9;
    ul_uint32_t r10;
    ul_uint32_t r11;

    struct {
        ul_uint32_t r0;
        ul_uint32_t r1;
        ul_uint32_t r2;
        ul_uint32_t r3;
        ul_uint32_t r12;
        ul_uint32_t lr;
        ul_uint32_t pc;
        ul_uint32_t psr;
    }exception_stack_frame;
};

/* 中断处理相关标志 */
ul_uint32_t ul_interrupt_from_thread_sp, ul_interrupt_to_thread_sp;
ul_uint32_t ul_thread_switch_interrupt_flag;

/**
 * @brief 初始化线程栈
 *
 * @param tentry 线程入口函数
 * @param parameter 线程参数
 * @param stack_addr 栈顶地址
 * @param texit 线程退出函数
 * @return ul_uint8_t* 初始化后的栈指针
 */
ul_uint8_t *ul_hw_stack_init(void       *tentry,
                             void       *parameter,
                             ul_uint8_t *stack_addr,
                             void       *texit)
{
    struct stack_frame *stack_frame;
    ul_uint8_t         *stk;
    unsigned long       i;

    stk  = stack_addr;
    stk  = (ul_uint8_t *)UL_ALIGN_DOWN((ul_uint32_t)stk, 8);
    stk -= sizeof(struct stack_frame);

    stack_frame = (struct stack_frame *)stk;

    /* 初始化所有寄存器 */
    for (i = 0; i < sizeof(struct stack_frame) / sizeof(ul_uint32_t); i ++)
    {
        ((ul_uint32_t *)stack_frame)[i] = 0xdeadbeef;
    }

    /* 设置初始寄存器值 */
    stack_frame->exception_stack_frame.r0  = (unsigned long)parameter; /* r0: 参数 */
    stack_frame->exception_stack_frame.lr  = (unsigned long)texit;     /* lr: 退出函数 */
    stack_frame->exception_stack_frame.pc  = (unsigned long)tentry;    /* pc: 入口函数 */
    stack_frame->exception_stack_frame.psr = 0x01000000L;              /* PSR: Thumb模式 */
    
    /* 
     * 栈初始化后的内存布局（栈增长方向向下）：
     * 
     * 高地址
     * +-------------------+ <- stack_addr (原始栈顶)
     * |   ...             |
     * +-------------------+
     * |   flag            | <- 如果启用FPU
     * +-------------------+
     * |   R4              |
     * +-------------------+
     * |   R5              |
     * +-------------------+
     * |   R6              |
     * +-------------------+
     * |   R7              |
     * +-------------------+
     * |   R8              |
     * +-------------------+
     * |   R9              |
     * +-------------------+
     * |   R10             |
     * +-------------------+
     * |   R11             |
     * +-------------------+
     * |   xPSR (0x01000000L)|
     * +-------------------+
     * |   PC (tentry)     |
     * +-------------------+
     * |   LR (texit)      |
     * +-------------------+
     * |   R12             |
     * +-------------------+
     * |   R3              |
     * +-------------------+
     * |   R2              |
     * +-------------------+
     * |   R1              |
     * +-------------------+
     * |   R0 (parameter)  |
     * +-------------------+ <- stk (返回的新栈顶)
     * 低地址
     */

    return stk;
}
