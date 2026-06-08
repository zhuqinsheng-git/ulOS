#include "ul_config.h"

/**
 * 检测编译器和目标平台是否支持硬件FPU
 * 支持以下编译器组合：
 * - ARMCC with VFP
 * - Clang with VFP and not SOFTFP
 * - IAR with ARMVFP
 * - GCC with VFP and not SOFTFP
 */
#if               /* ARMCC */ (  (defined ( __CC_ARM ) && defined ( __TARGET_FPU_VFP ))    \
                  /* Clang */ || (defined ( __clang__ ) && defined ( __VFP_FP__ ) && !defined(__SOFTFP__)) \
                  /* IAR */   || (defined ( __ICCARM__ ) && defined ( __ARMVFP__ ))        \
                  /* GNU */   || (defined ( __GNUC__ ) && defined ( __VFP_FP__ ) && !defined(__SOFTFP__)) )
#define USE_FPU   1    /**< 启用FPU支持 */
#else
#define USE_FPU   0    /**< 禁用FPU支持 */
#endif

/**
 * @brief 异常发生时的栈帧结构
 * 
 * 该结构包含了ARM Cortex-M在异常发生时自动保存的寄存器
 */
struct exception_stack_frame
{
    ul_uint32_t r0;
    ul_uint32_t r1;
    ul_uint32_t r2;
    ul_uint32_t r3;
    ul_uint32_t r12;
    ul_uint32_t lr;   /**< 链接寄存器 (R14) */
    ul_uint32_t pc;   /**< 程序计数器 (R15) */
    ul_uint32_t psr;  /**< 程序状态寄存器 */
};

/**
 * @brief 完整的线程栈帧结构
 * 
 * 包含了需要手动保存的寄存器(R4-R11)和异常栈帧
 */
struct stack_frame
{
#if USE_FPU
    ul_uint32_t flag; /**< FPU使用标志 */
#endif /* USE_FPU */

    /* r4 ~ r11 register */
    ul_uint32_t r4;
    ul_uint32_t r5;
    ul_uint32_t r6;
    ul_uint32_t r7;
    ul_uint32_t r8;
    ul_uint32_t r9;
    ul_uint32_t r10;
    ul_uint32_t r11;

    struct exception_stack_frame exception_stack_frame; /**< 异常栈帧 */
};

/**
 * @brief 支持FPU的异常栈帧结构
 * 
 * 包含基本寄存器和FPU寄存器(S0-S15)
 */
struct exception_stack_frame_fpu
{
    ul_uint32_t r0;
    ul_uint32_t r1;
    ul_uint32_t r2;
    ul_uint32_t r3;
    ul_uint32_t r12;
    ul_uint32_t lr;   /**< 链接寄存器 (R14) */
    ul_uint32_t pc;   /**< 程序计数器 (R15) */
    ul_uint32_t psr;  /**< 程序状态寄存器 */

#if USE_FPU
    /* FPU register */
    ul_uint32_t S0;   /**< FPU寄存器 S0 */
    ul_uint32_t S1;   /**< FPU寄存器 S1 */
    ul_uint32_t S2;   /**< FPU寄存器 S2 */
    ul_uint32_t S3;   /**< FPU寄存器 S3 */
    ul_uint32_t S4;   /**< FPU寄存器 S4 */
    ul_uint32_t S5;   /**< FPU寄存器 S5 */
    ul_uint32_t S6;   /**< FPU寄存器 S6 */
    ul_uint32_t S7;   /**< FPU寄存器 S7 */
    ul_uint32_t S8;   /**< FPU寄存器 S8 */
    ul_uint32_t S9;   /**< FPU寄存器 S9 */
    ul_uint32_t S10;  /**< FPU寄存器 S10 */
    ul_uint32_t S11;  /**< FPU寄存器 S11 */
    ul_uint32_t S12;  /**< FPU寄存器 S12 */
    ul_uint32_t S13;  /**< FPU寄存器 S13 */
    ul_uint32_t S14;  /**< FPU寄存器 S14 */
    ul_uint32_t S15;  /**< FPU寄存器 S15 */
    ul_uint32_t FPSCR;/**< 浮点状态和控制寄存器 */
    ul_uint32_t NO_NAME; /**< 保留字段 */
#endif
};

/**
 * @brief 支持FPU的完整线程栈帧结构
 * 
 * 包含所有需要保存的寄存器和FPU状态
 */
struct stack_frame_fpu
{
    ul_uint32_t flag; /**< FPU使用标志 */

    /* r4 ~ r11 register */
    ul_uint32_t r4;
    ul_uint32_t r5;
    ul_uint32_t r6;
    ul_uint32_t r7;
    ul_uint32_t r8;
    ul_uint32_t r9;
    ul_uint32_t r10;
    ul_uint32_t r11;

#if USE_FPU
    /* FPU register s16 ~ s31 */
    ul_uint32_t s16;  /**< FPU寄存器 S16 */
    ul_uint32_t s17;  /**< FPU寄存器 S17 */
    ul_uint32_t s18;  /**< FPU寄存器 S18 */
    ul_uint32_t s19;  /**< FPU寄存器 S19 */
    ul_uint32_t s20;  /**< FPU寄存器 S20 */
    ul_uint32_t s21;  /**< FPU寄存器 S21 */
    ul_uint32_t s22;  /**< FPU寄存器 S22 */
    ul_uint32_t s23;  /**< FPU寄存器 S23 */
    ul_uint32_t s24;  /**< FPU寄存器 S24 */
    ul_uint32_t s25;  /**< FPU寄存器 S25 */
    ul_uint32_t s26;  /**< FPU寄存器 S26 */
    ul_uint32_t s27;  /**< FPU寄存器 S27 */
    ul_uint32_t s28;  /**< FPU寄存器 S28 */
    ul_uint32_t s29;  /**< FPU寄存器 S29 */
    ul_uint32_t s30;  /**< FPU寄存器 S30 */
    ul_uint32_t s31;  /**< FPU寄存器 S31 */
#endif

    struct exception_stack_frame_fpu exception_stack_frame; /**< 异常栈帧 */
};

/* 中断处理相关全局变量 */
ul_uint32_t ul_interrupt_from_thread_sp;   /**< 中断发生时的线程栈指针 */
ul_uint32_t ul_interrupt_to_thread_sp;     /**< 中断返回时的线程栈指针 */
ul_uint32_t ul_thread_switch_interrupt_flag; /**< 线程切换中断标志 */

/**
 * @brief 初始化线程硬件栈
 * 
 * @param tentry 线程入口函数指针
 * @param parameter 传递给线程的参数
 * @param stack_addr 栈顶地址
 * @param texit 线程退出函数指针
 * @return ul_uint8_t* 初始化后的栈指针
 */
ul_uint8_t *ul_hw_stack_init(void       *tentry,
                             void       *parameter,
                             ul_uint8_t *stack_addr,
                             void       *texit)
{
    struct stack_frame *stack_frame;  /**< 栈帧结构指针 */
    ul_uint8_t         *stk;         /**< 栈指针 */
    unsigned long       i;           /**< 循环计数器 */

    /* 对齐栈地址到8字节边界 */
    stk = stack_addr;
    stk = (ul_uint8_t *)UL_ALIGN_DOWN((ul_uint32_t)stk, 8);
    
    /* 分配栈帧空间 */
    stk -= sizeof(struct stack_frame);
    stack_frame = (struct stack_frame *)stk;

    /* 初始化所有寄存器为0xdeadbeef */
    for (i = 0; i < sizeof(struct stack_frame) / sizeof(ul_uint32_t); i ++)
    {
        ((ul_uint32_t *)stack_frame)[i] = 0xdeadbeef;
    }

    /* 设置异常栈帧 */
    stack_frame->exception_stack_frame.r0  = (unsigned long)parameter; /* r0: 参数 */
    stack_frame->exception_stack_frame.r1  = 0;                        /* r1 */
    stack_frame->exception_stack_frame.r2  = 0;                        /* r2 */
    stack_frame->exception_stack_frame.r3  = 0;                        /* r3 */
    stack_frame->exception_stack_frame.r12 = 0;                        /* r12 */
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
     
#if USE_FPU
    stack_frame->flag = 0;  /**< 初始化FPU标志 */
#endif /* USE_FPU */

    return stk;  /**< 返回新的栈顶指针 */
}
