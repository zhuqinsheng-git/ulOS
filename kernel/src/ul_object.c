/*
 * Copyright (c) 2025 ulOS Community
 *
 * SPDX-License-Identifier: GPL-2.0-or-late
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-11-01     zhuqinsheng   the first version
 * 2025-12-3      zhuqinsheng   优化对象名字查找
 */
#include "ul_object.h"
#include "ul_libc.h"
/* Static global object list */

struct ul_object_info
{
    ul_object_class_type type;                     /**< object class type */
    ul_list_t            list;              /**< object list */
};

static struct ul_object_info g_object_container[UL_OBJECT_CLASS_MAX] =
{
{UL_OBJECT_CLASS_UNKNOWN, UL_LIST_HEAD_INIT(g_object_container[UL_OBJECT_CLASS_UNKNOWN].list)},
{UL_OBJECT_CLASS_THREAD, UL_LIST_HEAD_INIT(g_object_container[UL_OBJECT_CLASS_THREAD].list)},
#if (ULOS_CONFIG_USE_MUTEX != 0)
{UL_OBJECT_CLASS_MUTEX, UL_LIST_HEAD_INIT(g_object_container[UL_OBJECT_CLASS_MUTEX].list)},
#endif
#if (ULOS_CONFIG_USE_QUEUE != 0)
{UL_OBJECT_CLASS_QUEUE, UL_LIST_HEAD_INIT(g_object_container[UL_OBJECT_CLASS_QUEUE].list)},
#endif
#if (ULOS_CONFIG_USE_SEMAPHORE != 0)
{UL_OBJECT_CLASS_SEMAPHORE, UL_LIST_HEAD_INIT(g_object_container[UL_OBJECT_CLASS_SEMAPHORE].list)},
#endif
#if (ULOS_CONFIG_USE_EVENT != 0)
{UL_OBJECT_CLASS_EVENT, UL_LIST_HEAD_INIT(g_object_container[UL_OBJECT_CLASS_EVENT].list)},
#endif
#if (ULOS_CONFIG_USE_TIMER != 0)
{UL_OBJECT_CLASS_TIMER, UL_LIST_HEAD_INIT(g_object_container[UL_OBJECT_CLASS_TIMER].list)},
#endif
{UL_OBJECT_CLASS_SWTIMER, UL_LIST_HEAD_INIT(g_object_container[UL_OBJECT_CLASS_SWTIMER].list)},
#if (ULOS_CONFIG_USE_TOPIC != 0)
{UL_OBJECT_CLASS_TOPIC, UL_LIST_HEAD_INIT(g_object_container[UL_OBJECT_CLASS_TOPIC].list)},
#endif
{UL_OBJECT_CLASS_PIN, UL_LIST_HEAD_INIT(g_object_container[UL_OBJECT_CLASS_PIN].list)},
};

/**
 * Register object to the global object list
 * @param   name: name of the object to register
 * @param   object: pointer to the object structure to register
 * @return  UL_EOK on success, UL_ERROR on failure
 */
ul_ecode ul_object_init(const char* name, ul_object_t* object, ul_object_class_type type)
{
    if (name == UL_NULL || object == UL_NULL)
    {
        return UL_ERROR;
    }

    /* Check name length */
    if (ul_strlen(name) >= sizeof(object->name))
    {
        return UL_ERROR;
    }

    /* Check if already registered */
    if (ul_object_find(name, type) != UL_NULL)
    {
        return UL_ERROR;
    }
    
    object->type = type;
    
    /* Set object properties */
    ul_strncpy(object->name, name, sizeof(object->name) - 1);
    object->name[sizeof(object->name) - 1] = '\0'; /* Ensure string termination */

    /* Initialize list node */
    ul_list_init(&object->node);

    /* Add to global list (insert at tail to maintain registration order) */
    ul_list_insert_before(&g_object_container[type].list, &object->node);

    return UL_EOK;
}

/**
 * Unregister object from the global object list
 * @param   object: pointer to the object structure to unregister
 * @return  UL_EOK on success, UL_ERROR on failure
 */
ul_ecode ul_object_unregister(ul_object_t* object)
{
    /* Parameter check */
    if (object == UL_NULL) {
        return UL_ERROR;
    }

    /* Check if object is in list (by checking if list node is initialized) */
    if (object->node.next == UL_NULL && object->node.prev == UL_NULL) {
        return UL_ERROR; /* Object not registered */
    }

    ul_list_del_init(&object->node);

    /* Clear object name (optional, for security) */
    object->name[0] = '\0';

    return UL_EOK;
}

/**
 * Find object in the global object list by name
 * @param   name: name of the object to find
 * @return  pointer to the found object structure, or NULL if not found
 */
ul_object_t* ul_object_find(const char* name, ul_object_class_type type)
{
    ul_list_t *pos;
    ul_object_t *object;

    if (name == UL_NULL)
    {
        return UL_NULL;
    }

    /* Traverse object list */
    ul_list_for_each(pos, &g_object_container[type].list)
    {
        object = ul_list_entry(pos, ul_object_t, node);

        /* Compare object name */
        if (ul_strncmp(object->name, name, UL_OBJECT_NAME_MAX_LENGTH - 1) == 0)
        {
            return object;
        }
    }

    return UL_NULL;
}

/**
 * Print all object names in the global object list using custom output function
 * @param   output_func: function pointer for output (like printf)
 * @return  UL_EOK on success
 */
ul_ecode ul_object_list(ul_output_func_t kprtinf)
{
    ul_list_t *pos;
    ul_object_t *object;
    uint32_t count = 0;

    kprtinf("===================================\r\n");
    kprtinf("=          ULIB 2025              =\r\n");
    kprtinf("===================================\r\n");
    kprtinf("=     _   _ _       _ ____        =\r\n");
    kprtinf("=    | | | | |     | |  _ \\       =\r\n");
    kprtinf("=    | | | | |     | | |_) |      =\r\n");
    kprtinf("=    | | | | |___  | |  _ <       =\r\n");
    kprtinf("=    | |_| |_____| | | |_| |      =\r\n");
    kprtinf("=     \\___/        |_|____/       =\r\n");
    kprtinf("===================================\r\n");
    kprtinf("=     C Programming Library       =\r\n");
    kprtinf("===================================\r\n");
    kprtinf("Compile Time: %s %s\r\n", __DATE__, __TIME__);
    
    kprtinf("\n== UL Object List ==\r\n");

    /* Traverse object list */
    for (int i = 0; i < UL_OBJECT_CLASS_MAX; i++)
    {
        ul_list_for_each(pos, &g_object_container[i].list)
        {
            object = ul_list_entry(pos, ul_object_t, node);
            kprtinf("[%u] %s\r\n", ++count, object->name);
        }
    }
    
    kprtinf("Total objects: %u\r\n", count);
    kprtinf("====================\r\n\n");
    
    return UL_EOK;
}

