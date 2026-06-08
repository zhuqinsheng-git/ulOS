/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-9-3     zhuqinsheng   the first version
 */
#ifndef _UL_BUZZER_H
#define _UL_BUZZER_H

#include "ul_config.h"
#include "ul_list.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*
 * buzzer type definition
 */
#define UL_BUZZER_TYPE_STATIC      0x01
#define UL_BUZZER_TYPE_DYNAMIC     0x02

#define TONE_P0 	0	// 休止符频率

#define TONE_C3  262
#define TONE_C3_ 277
#define TONE_D3  294
#define TONE_D3_ 311
#define TONE_E3  330
#define TONE_F3  349
#define TONE_F3_ 370 
#define TONE_G3  392
#define TONE_G3_ 415
#define TONE_A3  440
#define TONE_A3_ 466
#define TONE_B3  494

#define TONE_C4  523
#define TONE_C4_ 554
#define TONE_D4  587
#define TONE_D4_ 622
#define TONE_E4  659
#define TONE_F4  698
#define TONE_F4_ 740
#define TONE_G4  784
#define TONE_G4_ 831
#define TONE_A4  880
#define TONE_A4_ 932
#define TONE_B4  988

#define TONE_C5  1047
#define TONE_C5_ 1109
#define TONE_D5  1175
#define TONE_D5_ 1245
#define TONE_E5  1319
#define TONE_F5  1397
#define TONE_F5_ 1480
#define TONE_G5  1568
#define TONE_G5_ 1661
#define TONE_A5  1760
#define TONE_A5_ 1865
#define TONE_B5  1976

/*
 * buzzer structure
 */
typedef struct ul_buzzer ul_buzzer_t;
struct ul_buzzer 
{
	ul_uint8_t      id;
    ul_uint8_t      stage;
    ul_uint32_t     ticks_count;
    uint16_t        freq;
    struct 
    {
        ul_uint8_t  data_update;
        ul_uint8_t  repeat;
        ul_uint32_t ticks_set;
        ul_uint32_t ticks_clear;
    } ctrl;
    
    ul_list_t       node;
    
    ul_uint8_t      type;
    /* 移植接口 */
    /**
      * @brief 设置 buzzer设备状态 （例如LED亮灭，蜂鸣器开关）
      * @param new_state buzzer设备新的状态 
      * @attention 以LED为例，无论实际电路是高电平亮起还是低电平亮起，此函数都必须实现写1时亮起，写0时熄灭
     */
    void (*set_freq)(ul_buzzer_t* self, ul_uint16_t freq);
};
typedef void (*func_set_freq)(ul_buzzer_t* self, ul_uint16_t freq);
/*
 * buzzer interface
 *
 * 使用步骤
 * 1. ul_buzzer_init(&led[i], i, set_buzzer);
 * 2. 按照对象结构体内移植接口说明实现set_buzzer函数
 */

ul_ecode ul_buzzer_init(ul_buzzer_t * self, uint8_t id, func_set_freq set_buzzer);
ul_buzzer_t* ul_buzzer_create(uint8_t id, func_set_freq set_buzzer);
ul_ecode ul_buzzer_delete(ul_buzzer_t *self);

void ul_buzzer_timer_callback(ul_uint32_t period);

ul_ecode ul_buzzer_set(ul_buzzer_t * self, uint16_t freq);
ul_ecode ul_buzzer_clear(ul_buzzer_t *self);
ul_ecode ul_buzzer_pulse(ul_buzzer_t * self, uint16_t freq, ul_uint32_t ticks_set, ul_uint32_t ticks_clear, ul_uint32_t repeat);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif

