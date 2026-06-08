/*
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-01     zhuqinsheng   the first version, merge ul_list
 */
#ifndef UL_OBJECT_H
#define UL_OBJECT_H

#include "ul_config.h"
#include "ul_heap.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* ======================================================== */
/* ----------------------- LIST --------------------------- */
/* ======================================================== */
typedef struct ul_list_node
{
    struct ul_list_node *next;     /* point to next node */
    struct ul_list_node *prev;     /* point to prev node */
}ul_list_t;
   
#define UL_LIST_HEAD_INIT(name) \
    { &(name), &(name) }  

/**
 * @brief initialize a list
 *
 * @param l list to be initialized
 */
ul_inline void ul_list_init(ul_list_t *l)
{
    l->next = l->prev = l;
}

ul_inline void ul_list_remove(ul_list_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

/**
 * @brief tests whether a list is empty
 * @param l the list to test.
 */
ul_inline int ul_list_isempty(const ul_list_t *l)
{
    return l->next == l;
}

/**
 * Check if the list node is the last one in the list
 * @param list: the list node to check
 * @param head: the head of the list
 * @return true if it's the last node, false otherwise
 */
ul_inline ul_bool_t ul_list_is_last(const ul_list_t *list, const ul_list_t *head)
{
    return list->next == head;
}


/**
 * @brief insert a node after a list
 *
 * @param l list to insert it
 * @param n new node to be inserted
 */
ul_inline void ul_list_insert_after(ul_list_t *l, ul_list_t *n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}

/**
 * @brief insert a node before a list
 *
 * @param n new node to be inserted
 * @param l list to insert it
 */
ul_inline void ul_list_insert_before(ul_list_t *l, ul_list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}

/**
 * ul_list_for_each - iterate over a list
 * @pos:    the rt_list_t * to use as a loop cursor.
 * @head:   the head for your list.
 */
#define ul_list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

// 安全遍历宏（允许删除节点）
#define ul_list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

// 反向遍历宏
#define ul_list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

// 反向安全遍历宏
#define ul_list_for_each_prev_safe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev; pos != (head); \
         pos = n, n = pos->prev)

/**
 * @brief get the struct for this entry
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define ul_list_entry(node, type, member) \
    ((type *)((char *)(node) - (unsigned long)(&((type *)0)->member)))

// 遍历链表并获取包含结构体（安全版本）
#define ul_list_for_each_entry_safe(pos, n, head, member) \
    for (pos = ul_list_entry((head)->next, typeof(*pos), member), \
         n = ul_list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = n, n = ul_list_entry(n->member.next, typeof(*n), member))    



/* ======================================================== */
/* ----------------------- OBJECT ------------------------- */
/* ======================================================== */
#define UL_OBJECT_FLAG_STATIC      0x01
#define UL_OBJECT_FLAG_DYNAMIC     0x02

typedef enum
{
    UL_OBJECT_CLASS_UNKNOWN,
    UL_OBJECT_CLASS_THREAD = 0x01,
#if (ULOS_CONFIG_USE_MUTEX != 0)
    UL_OBJECT_CLASS_MUTEX,
#endif
#if (ULOS_CONFIG_USE_QUEUE != 0)
    UL_OBJECT_CLASS_QUEUE,
#endif
#if (ULOS_CONFIG_USE_SEMAPHORE != 0)
    UL_OBJECT_CLASS_SEMAPHORE,
#endif
#if (ULOS_CONFIG_USE_EVENT != 0)
    UL_OBJECT_CLASS_EVENT,
#endif
#if (ULOS_CONFIG_USE_TIMER != 0)
    UL_OBJECT_CLASS_TIMER,
#endif
    UL_OBJECT_CLASS_SWTIMER,
#if (ULOS_CONFIG_USE_TOPIC != 0)
    UL_OBJECT_CLASS_TOPIC,
#endif
    UL_OBJECT_CLASS_PIN,
    UL_OBJECT_CLASS_MAX
} ul_object_class_type;
    
// 对象基类
typedef struct ul_object 
{
    char                        name[UL_OBJECT_NAME_MAX_LENGTH];
    ul_uint8_t                  flag;
    ul_object_class_type        type;
    ul_list_t                   node;
} ul_object_t;

// 对象管理接口
ul_ecode ul_object_init(const char* name, ul_object_t* object, ul_object_class_type type);
ul_ecode ul_object_unregister(ul_object_t* object);
ul_object_t* ul_object_find(const char* name, ul_object_class_type type);

typedef int (*ul_output_func_t)(const char *format, ...);
ul_ecode ul_object_list(ul_output_func_t kprtinf);


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif

