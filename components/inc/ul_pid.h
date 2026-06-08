/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *_
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-12     zhuqinsheng   the first version
 * 2025-9-01     zhuqinsheng   修改了一些程序BUG，防止除数为0的情况
 */
#ifndef _UL_PID_H
#define _UL_PID_H

#include "ul_config.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */
    
/*
 * pid type definitions
 */
#define UL_PID_MODE_DELTA          0  // 增量式PID
#define UL_PID_MODE_POSITION       1  // 位置式PID

/**
 * pid structure
 */
struct ul_pid
{
    /* 用户PID参数 */
    struct
    {
        ul_uint8_t mode;       // PID模式
        
        ul_fp32_t kp;
        ul_fp32_t ki;
        ul_fp32_t kd; 
        
        ul_fp32_t target;
        
        ul_fp32_t iout_limit;   // PID积分限幅值
        ul_fp32_t output_limit; // PID输出限幅值
    }user;
    /* PID过程数据 */
    struct
    {
        ul_fp32_t error[3];     // 误差项，[0]最新 [1]上一次 [2]上上次
        ul_fp32_t error_sum;    // PID误差累积值
    }process;
    /* PID输出数据 */
    struct
    {
        ul_fp32_t pout;         
        ul_fp32_t iout;
        ul_fp32_t dout;
        ul_fp32_t output;
    }outs;
};
typedef struct ul_pid ul_pid_t;


/**
 * PID参数修改
 *
 * @param self          PID对象
 * @param target        PID目标值
 *
 */
#define ul_pid_set_param(self, pid_param) \
{                                         \
    self.user.kp = pid_param[0];          \
    self.user.ki = pid_param[1];          \
    self.user.kd = pid_param[2];          \
}

/**
 * PID目标值修改
 *
 * @param self          PID对象
 * @param target        PID目标值
 *
 */
#define ul_pid_set_target(self, ptarget) \
{                                        \
    self.user.target = ptarget;          \
}

/**
 * PID积分限幅修改
 *
 * @param self          PID对象
 * @param iout_limit    PID积分限幅值
 *
 */
#define ul_pid_set_iout_limit(self, piout_limit)         \
{                                                        \
    self.user.iout_limit = piout_limit / self->user.ki;  \
}

/**
 * PID输出限幅修改
 *
 * @param self          PID对象
 * @param iout_limit    PID输出限幅值
 *
 */
#define ul_pid_set_output_limit(self, poutput_limit) \
{                                                    \
    self.user.output_limit = poutput_limit;          \
}

/*
 * pid interface
 */
ul_ecode ul_pid_init(ul_pid_t *self, ul_uint8_t mode, ul_fp32_t pid_param[3], ul_fp32_t iout_limit, ul_fp32_t output_limit, ul_fp32_t target);
ul_ecode ul_pid_update(ul_pid_t *self, ul_fp32_t feedback);


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
