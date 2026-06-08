/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-8-12     zhuqinsheng   the first version
 * 2025-9-07     zhuqinsheng   将擦除和写入分开
 */
#include "ul_w25qxx.h"

#define W25QXX_WriteEnable		      0x06
#define W25QXX_WriteDisable		      0x04
#define W25QXX_ReadStatusReg		  0x05
#define W25QXX_WriteStatusReg		  0x01
#define W25QXX_ReadData			      0x03
#define W25QXX_FastReadData		      0x0B
#define W25QXX_FastReadDual		      0x3B
#define W25QXX_PageProgram		      0x02
#define W25QXX_BlockErase			  0xD8
#define W25QXX_SectorErase		      0x20
#define W25QXX_ChipErase			  0xC7
#define W25QXX_PowerDown			  0xB9
#define W25QXX_ReleasePowerDown	      0xAB
#define W25QXX_DeviceID			      0xAB
#define W25QXX_ManufactDeviceID   	  0x90
#define W25QXX_JedecDeviceID		  0x9F

//读取芯片ID
//返回值如下:
//0XEF13,表示芯片型号为W25Q80
//0XEF14,表示芯片型号为W25Q16
//0XEF15,表示芯片型号为W25Q32
//0XEF16,表示芯片型号为W25Q64
//0XEF17,表示芯片型号为W25Q128
//读取设备ID
ul_uint16_t ul_w25qxx_checkid(ul_dev_w25qxx_t *self)
{
    ul_uint16_t  temp = 0;
    self->cs_write(0);
    //发送指令90h
    self->spi_rwbyte(0x90);//发送读取ID命令
    //发送地址  000000H
    self->spi_rwbyte(0x00);
    self->spi_rwbyte(0x00);
    self->spi_rwbyte(0x00);

    //接收制造商ID
    temp |= self->spi_rwbyte(0xFF) << 8;
    //接收设备ID
    temp |= self->spi_rwbyte(0xFF);
    self->cs_write(1);
    //返回ID
    return temp;
}

//发送写使能
static void ul_w25qxx_write_enable(ul_dev_w25qxx_t *self)
{
    self->cs_write(0);
    self->spi_rwbyte(0x06);
    self->cs_write(1);
}

static void ul_w25qxx_wait_busy(ul_dev_w25qxx_t *self)
{
    unsigned char byte = 0;

    do
    {
        self->cs_write(0);
        //发送指令05h
        self->spi_rwbyte(0x05);
        //接收状态寄存器值
        byte = self->spi_rwbyte(0Xff);
        self->cs_write(1);
        //判断BUSY位是否为1 如果为1说明在忙，重新读写BUSY位直到为0
    }
    while ( ( byte & 0x01 ) == 1 );
}

/**********************************************************
 * 函 数 名 称：w25qxx_erase_sector
 * 函 数 功 能：擦除一个扇区
 * 传 入 参 数：addr=擦除的扇区号
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：addr=擦除的扇区号，范围=0~15
**********************************************************/
static void w25qxx_erase_sector(ul_dev_w25qxx_t *self, ul_uint32_t addr)
{
    //计算扇区号，一个扇区4KB=4096
    addr *= 4096;
    ul_w25qxx_write_enable(self);  //写使能
    ul_w25qxx_wait_busy(self);     //判断忙，如果忙则一直等待
    self->cs_write(0);
    //发送指令20h
    self->spi_rwbyte(0x20);
    //发送24位扇区地址的高8位
    self->spi_rwbyte((ul_uint8_t)((addr) >> 16));
    //发送24位扇区地址的中8位
    self->spi_rwbyte((ul_uint8_t)((addr) >> 8));
    //发送24位扇区地址的低8位
    self->spi_rwbyte((ul_uint8_t)addr);
    self->cs_write(1);
    //等待擦除完成
    ul_w25qxx_wait_busy(self);
}

#define NORFLASH_SECTOR_SIZE 4096
void ul_w25qxx_erase(ul_dev_w25qxx_t *self, ul_uint32_t erase_addr, ul_uint32_t datalen)
{
    if (datalen == 0)
    {
        return;
    }

    ul_uint8_t sectorNum;
    ul_uint8_t addrOffset = erase_addr % NORFLASH_SECTOR_SIZE; 	// mod运算求余在一扇区内的偏移，若erase_addr是NORFLASH_SECTOR_SIZE整数倍，运算结果为0

    if (datalen > (NORFLASH_SECTOR_SIZE - addrOffset))      // 跨扇区
    {
        w25qxx_erase_sector(self, erase_addr / 4096);               // 擦本扇区
        erase_addr += NORFLASH_SECTOR_SIZE - addrOffset;   // 对齐到扇区地址
        datalen -= NORFLASH_SECTOR_SIZE - addrOffset;
        sectorNum = datalen / NORFLASH_SECTOR_SIZE;

        while (sectorNum--)
        {
            w25qxx_erase_sector(self, erase_addr / 4096);
            erase_addr += NORFLASH_SECTOR_SIZE;
        }

        if (datalen % NORFLASH_SECTOR_SIZE != 0)
        {
            w25qxx_erase_sector(self, erase_addr / 4096);
        }
    }
    else  // 没有跨扇区
    {
        w25qxx_erase_sector(self, erase_addr / 4096);
    }
}

/**********************************************************
 * 函 数 名 称：w25qxx_read
 * 函 数 功 能：读取W25Q32的数据
 * 传 入 参 数：buffer=读出数据的保存地址  read_addr=读取地址   read_length=读去长度
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
**********************************************************/
static void w25qxx_read(ul_dev_w25qxx_t *self, ul_uint8_t* buffer, ul_uint32_t read_addr, ul_uint16_t read_length)
{
    ul_uint16_t i;
    self->cs_write(0);
    //发送指令03h
    self->spi_rwbyte(0x03);
    //发送24位读取数据地址的高8位
    self->spi_rwbyte((ul_uint8_t)((read_addr) >> 16));
    //发送24位读取数据地址的中8位
    self->spi_rwbyte((ul_uint8_t)((read_addr) >> 8));
    //发送24位读取数据地址的低8位
    self->spi_rwbyte((ul_uint8_t)read_addr);

    //根据读取长度读取出地址保存到buffer中
    for (i = 0; i < read_length; i++)
    {
        buffer[i] = self->spi_rwbyte(0XFF);
    }

    self->cs_write(1);
}

static void ul_w25qxx_program_page(ul_dev_w25qxx_t *self, ul_uint32_t addr, ul_uint8_t *pBuffer, ul_uint32_t datalen)
{
    ul_w25qxx_write_enable(self);
    self->cs_write(0);
    self->spi_rwbyte(W25QXX_PageProgram);
    self->spi_rwbyte((addr & 0xFF0000) >> 16);
    self->spi_rwbyte((addr & 0x00FF00) >> 8);
    self->spi_rwbyte(addr & 0xFF);

    while (datalen--)
    {
        self->spi_rwbyte(*pBuffer);
        pBuffer++;
    }

    self->cs_write(1);
    ul_w25qxx_wait_busy(self);
}

#define PAGE_SIZE   256u//页空间256Byte
void ul_w25qxx_write(ul_dev_w25qxx_t *self, ul_uint32_t memaddr, ul_uint8_t *pdata, ul_uint16_t datalen)
{
    ul_uint16_t temp;
    
    int num = (memaddr % PAGE_SIZE) + datalen;
    num = num - PAGE_SIZE;//是否需要“跨”页写

    if (num < 1) //当页可以写完
    {
        ul_w25qxx_program_page(self, memaddr, pdata, datalen);
    }
    else //跨页
    {
        while (datalen)
        {
            temp = PAGE_SIZE - (memaddr % PAGE_SIZE); //确定当页可写的数量,最大256字节

            if (datalen >= temp)
            {
                ul_w25qxx_program_page(self, memaddr, pdata, temp);
                pdata += temp;
                datalen -= temp;
                memaddr += temp;
            }
            else if (datalen < temp)
            {
                ul_w25qxx_program_page(self, memaddr, pdata, datalen);
                pdata += datalen;
                datalen -= datalen;
                memaddr += datalen;
            }
        }
    }
}

void ul_w25qxx_read(ul_dev_w25qxx_t *self, ul_uint32_t memaddr, ul_uint8_t *pdata, ul_uint16_t datalen)
{
    ul_uint16_t temp;

    int num = (memaddr % PAGE_SIZE) + datalen;
    num = num - PAGE_SIZE;//是否需要“跨”页读

    if (num < 1) //当页可以读取完
    {
        w25qxx_read(self, pdata, memaddr, datalen);
        pdata += datalen;
    }
    else //跨页
    {
        while (datalen)
        {
            temp = PAGE_SIZE - (memaddr % PAGE_SIZE); //确定当页可读的数量,最大256字节

            if (datalen >= temp)
            {
                w25qxx_read(self, pdata, memaddr, datalen); //将此页读取完
                pdata += temp;
                datalen -= temp;
                memaddr += temp;
            }
            else if (datalen < temp)
            {
                w25qxx_read(self, pdata, memaddr, datalen); //将此页读取完
                pdata += datalen;
                datalen = 0;
                memaddr += datalen;
            }
        }
    }
}

ul_ecode ul_w25qxx_init(ul_dev_w25qxx_t *self, ul_uint8_t xx)
{
    if (self == NULL)
    {
        return UL_ENULL;
    }

    self->cs_write(1);
    self->xx = xx;
    
    return UL_EOK;
}
