/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-3      zhuqinsheng   the first version
 */
#ifndef _UL_LIBC_H
#define _UL_LIBC_H

#include "ul_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 内存操作函数 ==================== */

/**
 * @brief 内存拷贝
 * @param dest 目标内存地址
 * @param src 源内存地址  
 * @param n 拷贝字节数
 * @return 目标内存地址
 */
void *ul_memcpy(void *dest, const void *src, ul_size_t n);

/**
 * @brief 内存移动（处理重叠区域）
 * @param dest 目标内存地址
 * @param src 源内存地址
 * @param n 移动字节数
 * @return 目标内存地址
 */
void *ul_memmove(void *dest, const void *src, ul_size_t n);

/**
 * @brief 内存设置
 * @param s 内存地址
 * @param c 设置的值
 * @param n 设置字节数
 * @return 内存地址
 */
void *ul_memset(void *s, int c, ul_size_t n);

/**
 * @brief 内存比较
 * @param s1 内存块1
 * @param s2 内存块2
 * @param n 比较字节数
 * @return 相等返回0，s1>s2返回>0，s1<s2返回<0
 */
int ul_memcmp(const void *s1, const void *s2, ul_size_t n);

/**
 * @brief 内存查找
 * @param s 内存地址
 * @param c 查找的字符
 * @param n 查找范围字节数
 * @return 找到返回位置指针，未找到返回NULL
 */
void *ul_memchr(const void *s, int c, ul_size_t n);

/* ==================== 字符串操作函数 ==================== */

/**
 * @brief 字符串拷贝
 * @param dest 目标字符串
 * @param src 源字符串
 * @return 目标字符串
 */
char *ul_strcpy(char *dest, const char *src);

/**
 * @brief 字符串拷贝（限制长度）
 * @param dest 目标字符串
 * @param src 源字符串
 * @param n 最大拷贝字符数
 * @return 目标字符串
 */
char *ul_strncpy(char *dest, const char *src, ul_size_t n);

/**
 * @brief 字符串连接
 * @param dest 目标字符串
 * @param src 源字符串
 * @return 目标字符串
 */
char *ul_strcat(char *dest, const char *src);

/**
 * @brief 字符串连接（限制长度）
 * @param dest 目标字符串
 * @param src 源字符串
 * @param n 最大连接字符数
 * @return 目标字符串
 */
char *ul_strncat(char *dest, const char *src, ul_size_t n);

/**
 * @brief 字符串比较
 * @param s1 字符串1
 * @param s2 字符串2
 * @return 相等返回0，s1>s2返回>0，s1<s2返回<0
 */
int ul_strcmp(const char *s1, const char *s2);

/**
 * @brief 字符串比较（限制长度）
 * @param s1 字符串1
 * @param s2 字符串2
 * @param n 最大比较字符数
 * @return 相等返回0，s1>s2返回>0，s1<s2返回<0
 */
int ul_strncmp(const char *s1, const char *s2, ul_size_t n);

/**
 * @brief 字符串长度
 * @param s 字符串
 * @return 字符串长度（不含结束符）
 */
ul_size_t ul_strlen(const char *s);

/**
 * @brief 字符串查找字符
 * @param s 字符串
 * @param c 查找的字符
 * @return 找到返回位置指针，未找到返回NULL
 */
char *ul_strchr(const char *s, int c);

/**
 * @brief 字符串查找字符（从末尾）
 * @param s 字符串
 * @param c 查找的字符
 * @return 找到返回位置指针，未找到返回NULL
 */
char *ul_strrchr(const char *s, int c);

/**
 * @brief 字符串查找子串
 * @param haystack 主字符串
 * @param needle 子字符串
 * @return 找到返回位置指针，未找到返回NULL
 */
char *ul_strstr(const char *haystack, const char *needle);

/* ==================== 字符串/数字转换函数 ==================== */

/**
 * @brief 字符串转整数
 * @param nptr 数字字符串
 * @return 转换后的整数值
 */
int ul_atoi(const char *nptr);

/**
 * @brief 整数转字符串
 * @param value 整数值
 * @param str 输出缓冲区
 * @param base 进制（2-36）
 * @return 字符串指针
 */
char *ul_itoa(int value, char *str, int base);

/* ==================== 字符分类函数 ==================== */

/**
 * @brief 判断是否为数字字符
 * @param c 字符
 * @return 是数字返回1，否则返回0
 */
int ul_isdigit(int c);

/**
 * @brief 判断是否为字母字符
 * @param c 字符
 * @return 是字母返回1，否则返回0
 */
int ul_isalpha(int c);

/**
 * @brief 判断是否为字母或数字字符
 * @param c 字符
 * @return 是字母或数字返回1，否则返回0
 */
int ul_isalnum(int c);

/**
 * @brief 判断是否为空白字符
 * @param c 字符
 * @return 是空白字符返回1，否则返回0
 */
int ul_isspace(int c);

/**
 * @brief 字符转小写
 * @param c 字符
 * @return 小写字符
 */
int ul_tolower(int c);

/**
 * @brief 字符转大写
 * @param c 字符
 * @return 大写字符
 */
int ul_toupper(int c);

#ifdef __cplusplus
}
#endif

#endif /* UL_LIBC_H */
