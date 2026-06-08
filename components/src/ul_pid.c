/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-12     zhuqinsheng   the first version
 * 2025-9-01     zhuqinsheng   修改了一些程序BUG，防止除数为0的情况
 */
#include "ul_pid.h"

#define LIMIT_MIN_MAX(x, min, max) ((x) = (((x)<=(min))?(min):(((x)>=(max))?(max):(x))))	// 限幅

/**
 * PID初始化
 *
 * @param self          PID对象
 * @param mode          PID模式，[0]增量式，[1]位置式
 * @param pid_param     PID参数，pid_param[0]为kp，pid_param[1]为ki，pid_param[2]为kd
 * @param iout_limit    PID积分限幅值
 * @param output_limit  PID输出限幅值
 * @param target        PID目标值
 *
 * @return 错误代码
 */
ul_ecode ul_pid_init(ul_pid_t *self, ul_uint8_t mode, ul_fp32_t pid_param[3], ul_fp32_t iout_limit, ul_fp32_t output_limit, ul_fp32_t target)
{
    if (self == NULL)
    {
        return UL_ENULL;
    }
    
    self->user.mode         = mode;
    self->user.kp           = pid_param[0];
    self->user.ki           = pid_param[1];
    self->user.kd           = pid_param[2];
    if (self->user.ki != 0) // 防止除数为0
    {
        self->user.iout_limit   = iout_limit / self->user.ki;
    }
    else self->user.iout_limit = 0;
    self->user.output_limit = output_limit;
    self->user.target       = target;
    
    return UL_EOK;
}

/**
 * PID计算更新
 *
 * @param self          PID对象
 * @param feedback      PID反馈值
 *
 * @return 错误代码
 */
ul_ecode ul_pid_update(ul_pid_t *self, ul_fp32_t feedback)
{
    if (self == NULL)
    {
        return UL_ENULL;
    }
    
    self->process.error[2] = self->process.error[1];
    self->process.error[1] = self->process.error[0];
    self->process.error[0] = self->user.target - feedback;
    
    /* 位置式PID */
    if (self->user.mode == UL_PID_MODE_POSITION)
    {
        self->process.error_sum += self->process.error[0];
        LIMIT_MIN_MAX(self->process.error_sum, -self->user.iout_limit, self->user.iout_limit);
        self->outs.pout   = self->user.kp * self->process.error[0];
        self->outs.iout   = self->user.ki * self->process.error_sum;
        self->outs.dout   = self->user.kd * (self->process.error[0] - self->process.error[1]);
        self->outs.output = self->outs.pout + self->outs.iout + self->outs.dout;
        LIMIT_MIN_MAX(self->outs.output, -self->user.output_limit, self->user.output_limit);
    }
    /* 增量式PID */
    else if(self->user.mode == UL_PID_MODE_DELTA)
    {
        self->outs.pout   = self->user.kp * (self->process.error[0] - self->process.error[1]);
        self->outs.iout   = self->user.ki * self->process.error[0];
        self->outs.dout   = self->user.kd * (self->process.error[0] - 2.0f * self->process.error[1] + self->process.error[2]);
        self->outs.output = self->outs.pout + self->outs.iout + self->outs.dout;
        LIMIT_MIN_MAX(self->outs.output, -self->user.output_limit, self->user.output_limit);
    }
    else
    {
        return UL_ERROR;
    }

    return UL_EOK;
}

#if 0
    ul_pid_t pid;
    ul_fp32_t pid_param[3] = {1,0,0};
    ul_pid_init(&pid, 0, pid_param, 100, 100, 0);
    ul_pid_set_param(pid, pid_param);
    ul_pid_set_target(pid,0);
    ul_pid_set_output_limit(pid,200);
    ul_pid_update(&pid, 0);
#endif
