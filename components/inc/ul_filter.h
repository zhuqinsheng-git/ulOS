/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *_
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-7-20      zhuqinsheng   the first version
 */
#ifndef _UL_FILTER_H
#define _UL_FILTER_H

#include "ul_config.h"

// 低通滤波器结构体
typedef struct {
    float alpha;          // 滤波系数
    float prev_output;    // 上一次的输出
} ul_lowpass_filter_t;

// 初始化低通滤波器
void ul_lowpass_filter_init(ul_lowpass_filter_t* filter, float alpha);

// 应用低通滤波器
float ul_lowpass_filter_update(ul_lowpass_filter_t* filter, float input);

// 均值滤波器结构体
typedef struct {
    float* buffer;          // 用于存储采样值的缓冲区
    int buffer_size;        // 缓冲区大小
    int index;              // 当前索引
    float sum;              // 缓冲区中所有值的和
    int count;              // 当前已填充的样本数量
} MovingAverageFilter;

// 初始化均值滤波器
void MovingAverageFilter_Init(MovingAverageFilter* filter, float* buffer, int buffer_size);

// 应用均值滤波器
float MovingAverageFilter_Update(MovingAverageFilter* filter, float input);


// 定义滑动滤波器结构体
typedef struct {
    float* buffer;          // 存储滑动窗口数据的缓冲区
    uint32_t window_size;   // 滑动窗口的大小
    uint32_t index;         // 当前缓冲区的索引
    uint32_t count;         // 当前缓冲区中有效数据的数量
    float sum;              // 当前窗口内数据的总和
} ul_sliding_filter_t;

ul_ecode ul_sliding_filter_init(ul_sliding_filter_t* filter, uint32_t window_size);
float ul_sliding_filter_update(ul_sliding_filter_t* filter, float input);

#endif