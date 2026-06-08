/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-7-20      zhuqinsheng   the first version
 */
#include "ul_filter.h"
#include "ul_heap.h"
// 低通滤波示例代码
// int main(void) {
//     LowPassFilter myFilter;
//     float alpha = 0.1f;  // 滤波系数，可以根据需要调整

//     LowPassFilter_Init(&myFilter, alpha);

//     float input_values[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
//     float filtered_value;

//     for (int i = 0; i < 5; i++) {
//         filtered_value = LowPassFilter_Update(&myFilter, input_values[i]);
//         // 处理 filtered_value
//     }

//     return 0;
// }

// 初始化低通滤波器
void ul_lowpass_filter_init(ul_lowpass_filter_t* filter, float alpha)
{
    filter->alpha = alpha;
    filter->prev_output = 0.0f;  // 初始输出设为0
}

// 应用低通滤波器
float ul_lowpass_filter_update(ul_lowpass_filter_t* filter, float input)
{
    float output = filter->alpha * input + (1.0f - filter->alpha) * filter->prev_output;
    filter->prev_output = output;  // 更新上一次的输出
    return output;
}

// 初始化滑动滤波器
ul_ecode ul_sliding_filter_init(ul_sliding_filter_t* filter, uint32_t window_size)
{
    if (filter == NULL || window_size == 0)
    {
        return UL_ERROR; // 初始化失败，参数无效
    }

    filter->buffer = (float*)ul_malloc(sizeof(float) * window_size);

    if (filter->buffer == NULL)
    {
        return UL_ERROR; // 内存分配失败
    }

    for (uint32_t i = 0; i < window_size; i++)
    {
        filter->buffer[i] = 0.0f; // 初始化缓冲区为0
    }

    filter->window_size = window_size;
    filter->index = 0;
    filter->count = 0;
    filter->sum = 0.0f;

    return UL_EOK; // 初始化成功
}

// 更新滑动滤波器
float ul_sliding_filter_update(ul_sliding_filter_t* filter, float input)
{
    if (filter == NULL)
    {
        return 0.0f; // 无效的滤波器指针，返回0
    }

    if (filter->count < filter->window_size)
    {
        // 缓冲区尚未填满，累加输入值
        filter->sum += input;
        filter->buffer[filter->index] = input;
        filter->index = (filter->index + 1) % filter->window_size;
        filter->count += 1;
    }
    else
    {
        // 缓冲区已满，减去最早的值，累加新的输入值
        float oldest_value = filter->buffer[filter->index];
        filter->sum += input - oldest_value;
        filter->buffer[filter->index] = input;
        filter->index = (filter->index + 1) % filter->window_size;
    }

    // 计算并返回平均值
    return filter->sum / ((float)filter->count);
}

// 释放滑动滤波器占用的内存
void ul_sliding_filter_deinit(ul_sliding_filter_t* filter)
{
    if (filter != NULL && filter->buffer != NULL)
    {
        ul_free(filter->buffer);
        filter->buffer = NULL;
    }
}