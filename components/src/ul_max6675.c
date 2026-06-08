/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-21     zhuqinsheng   the first version
 */
#include "ul_max6675.h"

/**
 *  MAX6675初始化函数
 *
 * @param self        AT24CXX对象
 *
 * @return 错误代码
 */
ul_ecode ul_max6675_init(ul_max6675_t *self)
{
    if (self == UL_NULL)
    {
        return UL_ENULL;
    }
    
    return UL_EOK;
}

/**
 *  MAX6675读温度函数
 *
 * @param self        AT24CXX对象
 *
 * @return 错误代码，返回UL_ERROR代表K型热电偶未连接
 */
ul_ecode ul_max6675_update(ul_max6675_t *self)
{
    if (self == UL_NULL)
    {
        return UL_ENULL;
    }
    
    self->cs_write(0);
    
    ul_uint16_t rawdata = self->spi_rhalfword();
    
    self->cs_write(1);
    
    if (rawdata & 4)   // 第三位为0则热电偶未连接
    {
        self->temp = 0;
        return UL_ERROR;
    }
    
    self->temp = (rawdata >> 3) * 0.25f;
    
    return UL_EOK;
}
