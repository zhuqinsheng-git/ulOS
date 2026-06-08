/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-3      zhuqinsheng   the first version
 */
#include "ul_libc.h"

/* ==================== 内存操作函数实现 ==================== */

void *ul_memcpy(void *dest, const void *src, ul_size_t n)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;
    
    /* 逐字节拷贝 */
    for (ul_size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

void *ul_memmove(void *dest, const void *src, ul_size_t n)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;
    
    if (d == s) {
        return dest;  /* 源和目标相同 */
    }
    
    /* 检查重叠情况 */
    if (d > s && d < s + n) {
        /* 从后向前拷贝（正向重叠） */
        for (ul_size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    } else {
        /* 从前向后拷贝 */
        for (ul_size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    }
    
    return dest;
}

void *ul_memset(void *s, int c, ul_size_t n)
{
    unsigned char *p = (unsigned char *)s;
    unsigned char value = (unsigned char)c;
    
    for (ul_size_t i = 0; i < n; i++) {
        p[i] = value;
    }
    
    return s;
}

int ul_memcmp(const void *s1, const void *s2, ul_size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    
    for (ul_size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return (int)p1[i] - (int)p2[i];
        }
    }
    
    return 0;
}

void *ul_memchr(const void *s, int c, ul_size_t n)
{
    const unsigned char *p = (const unsigned char *)s;
    unsigned char value = (unsigned char)c;
    
    for (ul_size_t i = 0; i < n; i++) {
        if (p[i] == value) {
            return (void *)(p + i);
        }
    }
    
    return UL_NULL;
}

/* ==================== 字符串操作函数实现 ==================== */

char *ul_strcpy(char *dest, const char *src)
{
    char *d = dest;
    
    while ((*d++ = *src++) != '\0') {
        /* 空循环体 */
    }
    
    return dest;
}

char *ul_strncpy(char *dest, const char *src, ul_size_t n)
{
    char *d = dest;
    ul_size_t i;
    
    for (i = 0; i < n && src[i] != '\0'; i++) {
        d[i] = src[i];
    }
    
    /* 填充剩余空间为0 */
    for (; i < n; i++) {
        d[i] = '\0';
    }
    
    return dest;
}

char *ul_strcat(char *dest, const char *src)
{
    char *d = dest;
    
    /* 找到dest的结尾 */
    while (*d != '\0') {
        d++;
    }
    
    /* 追加src */
    while ((*d++ = *src++) != '\0') {
        /* 空循环体 */
    }
    
    return dest;
}

char *ul_strncat(char *dest, const char *src, ul_size_t n)
{
    char *d = dest;
    
    /* 找到dest的结尾 */
    while (*d != '\0') {
        d++;
    }
    
    /* 追加src（最多n个字符） */
    ul_size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        d[i] = src[i];
    }
    
    d[i] = '\0';  /* 确保以0结尾 */
    
    return dest;
}

int ul_strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int ul_strncmp(const char *s1, const char *s2, ul_size_t n)
{
    if (n == 0) {
        return 0;
    }
    
    while (--n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

ul_size_t ul_strlen(const char *s)
{
    const char *p = s;
    while (*p != '\0') {
        p++;
    }
    
    return (ul_size_t)(p - s);
}

char *ul_strchr(const char *s, int c)
{
    while (*s != '\0') {
        if (*s == (char)c) {
            return (char *)s;
        }
        s++;
    }
    
    return UL_NULL;
}

char *ul_strrchr(const char *s, int c)
{
    const char *last = UL_NULL;
    
    while (*s != '\0') {
        if (*s == (char)c) {
            last = s;
        }
        s++;
    }
    
    return (char *)last;
}

char *ul_strstr(const char *haystack, const char *needle)
{
    if (*needle == '\0') {
        return (char *)haystack;
    }
    
    for (; *haystack != '\0'; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        
        if (*n == '\0') {
            return (char *)haystack;
        }
    }
    
    return UL_NULL;
}

/* ==================== 字符串/数字转换函数实现 ==================== */

int ul_atoi(const char *nptr)
{
    int result = 0;
    int sign = 1;
    
    /* 跳过前导空白字符 */
    while (ul_isspace(*nptr)) {
        nptr++;
    }
    
    /* 处理符号 */
    if (*nptr == '-') {
        sign = -1;
        nptr++;
    } else if (*nptr == '+') {
        nptr++;
    }
    
    /* 转换数字 */
    while (ul_isdigit(*nptr)) {
        result = result * 10 + (*nptr - '0');
        nptr++;
    }
    
    return sign * result;
}

char *ul_itoa(int value, char *str, int base)
{
    char *ptr = str;
    char *start = str;
    char *end;
    int negative = 0;
    
    /* 处理0 */
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }
    
    /* 处理负数（只对10进制） */
    if (value < 0 && base == 10) {
        negative = 1;
        value = -value;
    }
    
    /* 转换数字（反向） */
    while (value != 0) {
        int remainder = value % base;
        *ptr++ = (remainder > 9) ? (remainder - 10) + 'a' : remainder + '0';
        value = value / base;
    }
    
    /* 添加负号 */
    if (negative) {
        *ptr++ = '-';
    }
    
    *ptr = '\0';
    
    /* 反转字符串 */
    end = ptr - 1;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    
    return str;
}

/* ==================== 字符分类函数实现 ==================== */

int ul_isdigit(int c)
{
    return (c >= '0' && c <= '9');
}

int ul_isalpha(int c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

int ul_isalnum(int c)
{
    return ul_isalpha(c) || ul_isdigit(c);
}

int ul_isspace(int c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
}

int ul_tolower(int c)
{
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

int ul_toupper(int c)
{
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}
