/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-xx     zhuqinsheng   the first version
 */
#include "ul_ina226.h"

#define INA226_CONFIG_REG                 0x00
#define INA226_SHUNT_V_REG                0x01
#define INA226_BUS_V_REG                  0x02
#define INA226_POWER_REG                  0x03
#define INA226_CURRENT_REG                0x04
#define INA226_CALIB_REG                  0x05
#define INA226_MASK_EN_REG                0x06
#define INA226_ALERT_REG                  0x07
#define INA226_MAN_ID_REG                 0xFE  //0x5449
#define INA226_ID_REG                     0xFF  //0x2260

static ul_ecode ina226_write2byte(ul_ina226_t *self, ul_uint8_t memaddr, ul_uint16_t wdata)
{
    if (self == UL_NULL)
    {
        return UL_ENULL;
    }
    
    uint8_t data_high=(uint8_t)((wdata&0xFF00)>>8);
	uint8_t data_low=(uint8_t)wdata&0x00FF;
    uint8_t wdata8[2];
    
    /* INA226为大端序 */
    wdata8[0] = data_high;  
    wdata8[1] = data_low;
    
    ul_ecode res = self->i2c_write(self, self->addr, memaddr, wdata8, 2);
    
    if (res != UL_EOK)
    {
        return res;
    }
    
    return UL_EOK;
}

static ul_ecode ina226_read2byte(ul_ina226_t *self, ul_uint8_t memaddr, ul_uint16_t *rdata)
{
    if (self == UL_NULL || rdata == UL_NULL)
    {
        return UL_ENULL;
    }
    
    ul_ecode res = self->i2c_read(self, self->addr, memaddr, (uint8_t *)rdata, 2);
    
    /* INA226为大端序 */
    *rdata = (*rdata << 8) | (*rdata >> 8);
    
    if (res != UL_EOK)
    {
        return res;
    }
    
    return UL_EOK;
}

static ul_ecode ul_ina226_set_config(ul_ina226_t *self, ul_uint16_t wdata)
{
    if (self == UL_NULL)
    {
        return UL_ENULL;
    }
    
    ul_ecode res;
    
    res = ina226_write2byte(self, INA226_CONFIG_REG, wdata);
    
    if (res != UL_EOK)
    {
        return res;
    }
    
    return UL_EOK;
}

/**
 * INA226初始化函数
 *
 * @param self        AT24CXX对象
 * @param addr        AT24CXX设备地址
 *
 * @return 错误代码
 */
ul_ecode ul_ina226_init(ul_ina226_t *self, ul_uint8_t addr, uint8_t average)
{
    if (self == NULL)
    {
        return UL_ENULL;
    }
    
    self->addr = addr;
    
    if (average > 7)
    {
        average = 0;
    }
    
    uint16_t config = 0x4127 | ((uint16_t)average << 9);
    
    ul_ina226_set_config(self, config);
    
    return UL_EOK;
}

ul_ecode ul_ina226_set_calibration(ul_ina226_t *self, float maximum_expected_current, float rshunt)
{
    if (self == UL_NULL)
    {
        return UL_ENULL;
    }
    
    ul_ecode res;
    
    float current_lsb = maximum_expected_current / (1 << 15);
    
    float cal = 0.00512f / (current_lsb * rshunt);
    
    uint16_t cal_16 = (uint16_t)cal;
    
    res = ina226_write2byte(self, INA226_CALIB_REG, cal_16);
    
    if (res != UL_EOK)
    {
        return res;
    }
    
    return UL_EOK;
}

ul_ecode ul_ina226_get_bus_voltage(ul_ina226_t *self, float *bus_voltage)
{
    if (self == UL_NULL)
    {
        return UL_ENULL;
    }
    
    ul_ecode res;
    
    int16_t rdata;
    
    res = ina226_read2byte(self, INA226_BUS_V_REG, (uint16_t *)&rdata);
    
    *bus_voltage = rdata * 1.25f;
    
    if (res != UL_EOK)
    {
        return res;
    }
    
    return UL_EOK;
}

ul_ecode ul_ina226_get_shunt_voltage(ul_ina226_t *self, int16_t *shunt_voltage)
{
    if (self == UL_NULL)
    {
        return UL_ENULL;
    }
    
    ul_ecode res;
    
    int16_t rdata;
    
    res = ina226_read2byte(self, INA226_SHUNT_V_REG, (uint16_t *)&rdata);
    
    *shunt_voltage = rdata / 4;
    
    if (res != UL_EOK)
    {
        return res;
    }
    
    return UL_EOK;
}
