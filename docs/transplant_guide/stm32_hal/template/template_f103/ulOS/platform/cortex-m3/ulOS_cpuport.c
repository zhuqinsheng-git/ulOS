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

/* flag in interrupt handling */
ul_uint32_t ul_interrupt_from_thread_sp, ul_interrupt_to_thread_sp;
ul_uint32_t ul_thread_switch_interrupt_flag;

ul_uint8_t *ul_hw_stack_init(void       *tentry,
                             void       *parameter,
                             ul_uint8_t *stack_addr,
                             void       *texit)
{
    struct stack_frame *stack_frame;
    ul_uint8_t         *stk;
    unsigned long       i;

    //stk  = stack_addr + sizeof(ul_uint32_t);
    stk  = stack_addr;
    stk  = (ul_uint8_t *)UL_ALIGN_DOWN((ul_uint32_t)stk, 8);
    stk -= sizeof(struct stack_frame);

    stack_frame = (struct stack_frame *)stk;

    /* init all register */
    for (i = 0; i < sizeof(struct stack_frame) / sizeof(ul_uint32_t); i ++)
    {
        ((ul_uint32_t *)stack_frame)[i] = 0xdeadbeef;
    }

    stack_frame->exception_stack_frame.r0  = (unsigned long)parameter; /* r0 : argument */
    stack_frame->exception_stack_frame.r1  = 0;                        /* r1 */
    stack_frame->exception_stack_frame.r2  = 0;                        /* r2 */
    stack_frame->exception_stack_frame.r3  = 0;                        /* r3 */
    stack_frame->exception_stack_frame.r12 = 0;                        /* r12 */
    stack_frame->exception_stack_frame.lr  = (unsigned long)texit;     /* lr */
    stack_frame->exception_stack_frame.pc  = (unsigned long)tentry;    /* entry point, pc */
    stack_frame->exception_stack_frame.psr = 0x01000000L;              /* PSR */
    
    /* 
     * 栈初始化后的内存布局（假设栈增长方向是向下）：
     * 
     * 高地址
     * +-------------------+ <- stack_start + stack_size (栈顶，初始SP位置)
     * |   xPSR            |
     * +-------------------+
     * |   PC (entry)      | <- entry函数指针
     * +-------------------+
     * |   LR (_thread_exit)|
     * +-------------------+
     * |   R12             |
     * +-------------------+
     * |   R3              |
     * +-------------------+
     * |   R2              |
     * +-------------------+
     * |   R1 (parameter)  | <- parameter参数指针
     * +-------------------+
     * |   R0              |
     * +-------------------+
     * |   R11             |
     * +-------------------+
     * |   R10             |
     * +-------------------+
     * |   R9              |
     * +-------------------+
     * |   R8              |
     * +-------------------+
     * |   R7              |
     * +-------------------+
     * |   R6              |
     * +-------------------+
     * |   R5              |
     * +-------------------+
     * |   R4              |
     * +-------------------+ <- stack_top
     * |   ...             |
     * +-------------------+
     * |   ...             |
     * +-------------------+
     * |   ###...###       | <- 用'#'填充的未使用空间
     * +-------------------+
     * |                   |
     * +-------------------+
     * 低地址
     * ^                   
     * |                   
     * stack_start        
     */
    return stk;
}

