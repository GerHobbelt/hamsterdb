/*
 * Copyright (C) 2005-2010 Christoph Rupp (chris@crupp.de).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 *
 * See files COPYING.* for License information.
 */

/**
* @cond ham_internals
*/
                       
/**
@brief A virtual device which provides event notification services for all device method invocations.

You can register callbacks with each method and all (or some) registered callbacks will be triggered
when that particular device method is invoked.

Depending on the registration flags, multiple callbacks can be registered as event listeners in a
series or broadcast fashion:

'series' callbacks share a common 'series ID' and are triggered one after the other until one of the
event listeners (callbacks) signals a event completion or event abort.

'broadcast' callbacks are invoked independently and in indeterminate order -- one should assume these
event listeners are invoked in parallel. The event trigger is completed when all listeners (callbacks)
have completed.
*/

#include "internal_preparation.h"



typedef enum
{
    HAM_EVENT_OPEN,
    HAM_EVENT_CLOSE,
    HAM_EVENT_DESTROY,
    HAM_EVENT_FLUSH,
    HAM_EVENT_READ,
    HAM_EVENT_WRITE,
    HAM_EVENT_READ_PAGE,
    HAM_EVENT_WRITE_PAGE,
    HAM_EVENT_SEEK,
    HAM_EVENT_TELL,
    HAM_EVENT_TRUNCATE,
    HAM_EVENT_ALLOC,
    HAM_EVENT_ALLOC_PAGE,

    HAM_EVENT_EXPAND,
    HAM_EVENT_SHRINK,

    HAM_EVENT_FREELIST_EXPAND,
    HAM_EVENT_FREELIST_ALLOC,
    HAM_EVENT_FREELIST_FREE,

    HAM_EVENT_CUSTOM = 32768, /* custom events from devices start numbering here... */
} ham_device_event_id_t;

typedef struct 
{
    int event_id;

    ham_device_invocation_t *caller;

    /* parameters */
    ham_offset_t offset;
    ham_offset_t *size;
    ham_page_t *page;
    void *buffer;

    int type; /* blob, btree page, freelist page, key space, record space */

    ham_key_t *key;
    ham_record_t *rec;

} ham_event_info_t;


/** 
generic function prototype for all and any event listeners (user defined callback functions) 

Single event listeners can be registered with multiple devices and/or action types, but the
event manager will trigger a HAM_EVENT_DESTROY event for each registration, i.e. the 
application programmer (a.k.a. user) is responsible for keeping a registration refcount 
around and only destroying event listener internal data when such refcount drops to zero,
that is: when the last HAM_EVENT_DESTROY event was fired ~ each registration has been matched 
by a HAM_EVENT_DESTROY event.
*/
typedef ham_status_t ham_device_action_event_cb_t(ham_event_info_t *info, int groupId, void *propagate);


typedef struct ham_listener_entry_t
{
    int groupId;

    ham_device_action_event_cb_t *listener;

    void *propagate;

} ham_listener_entry_t;

typedef struct ham_listener_list_t
{
    int event_id;

    ham_listener_entry_t *list;
    ham_size_t list_size;
    ham_size_t list_alloc_size;

} ham_listener_list_t;

typedef struct ham_listener_collective_t
{
    ham_listener_list_t *event_hash_table;
    ham_size_t table_size;

    /* the two primes, one for each hash function */
    int divisor[2];

    /** the memory allocator */
    mem_allocator_t *_malloc;

    ham_device_invocation_t *caller;

} ham_listener_collective_t;



/*
* get the allocator of this event list manager
*/
#define evtqueue_get_allocator(eq)          (eq)->_malloc

/*
* set the allocator of this event list manager
*/
#define evtqueue_set_allocator(eq, a)       (eq)->_malloc = (a)


ham_listener_collective_t *event_queue_new(ham_device_invocation_t *caller)
{
    ham_listener_collective_t *lst;
	mem_allocator_t *allocator = device_get_allocator(caller->me);

    lst = (ham_listener_collective_t *)allocator_calloc(allocator, sizeof(*lst));
    if (!lst)
        return NULL;

    evtqueue_set_allocator(lst, allocator);
    lst->caller = caller;

    return lst;
}

ham_status_t event_queue_destroy(ham_listener_collective_t *lst)
{
	mem_allocator_t *allocator = evtqueue_get_allocator(lst);

    if (lst->event_hash_table)
    {
        /* send each listener a HAM_EVENT_DESTROY event signal before we go */
        ham_size_t j;

        for (j = 0; j < lst->table_size; j++)
        {
            ham_listener_list_t *table = &lst->event_hash_table[j];

            if (table->list)
            {
                ham_size_t i;

                for (i = 0; i < table->list_size; i++)
                {
                    ham_event_info_t inf = {0};
                    ham_listener_entry_t *entry;

                    inf.event_id = HAM_EVENT_DESTROY;
                    inf.caller = lst->caller;
                    
                    entry = &table->list[i];
                    
                    if (entry->listener)
                        (void)entry->listener(&inf, entry->groupId, entry->propagate);
                }

                allocator_free(allocator, table->list);
            }
        }

        allocator_free(allocator, lst->event_hash_table);
    }

    allocator_free(allocator, lst);

    return HAM_SUCCESS;
}


/*
(bulk) register a callback:

groupid, callback, propagate, array of event ids to subscribe to
*/
ham_status_t event_queue_insert(ham_listener_collective_t *lst, 
                                int *actionIdSet, ham_size_t actionIdSet_size, 
                                ham_device_action_event_cb_t *listener, int groupId, void *propagate)
{

    return HAM_SUCCESS;
}


/**
Removal is a bit wicked as we support single event listeners which drive multiple targets themselves.

In other words: the unique 'key' towards each listener is really the tuple (listener, groupId, propagate),
but upon removal we tolerate 'partial keys' as long as those are unambiguous:
'NIL' fields are represented by zero(0) and the event manager seeks to remove any single entry which matches
all the non-zero fields; when such matching produces multiple hits, the 'partial key' is found to be
ambiguous and the removal function will return an error instead.
*/
ham_status_t event_queue_remove(ham_listener_collective_t *lst, 
                                int *actionIdSet, ham_size_t actionIdSet_size, 
                                ham_device_action_event_cb_t *listener, int groupId, void *propagate)
{

    return HAM_SUCCESS;
}






/*
one event list manages ALL event ids; one event list per HASHED event id.

This allows for easy expansion into arbitrary numbers of custom events while no need
to augment the event list manager in any way: event list manager / event device layer
should not need to know the exact number of event IDs for each custom device out there
so a generic solution like that is desirable.

Invocation is still fast as few actions map onto the same list.


Use cuckoo hashing to fill the table.

Redimension the table when more event ids show up; use two hash functions which are
mutual prime: two different prime numbers and a hashing which adds an arbitrary number
of times the size of the table to the id to be hashed, IFF a slot could not yet be found.

That way, each event id gets its own list and the only overhead is a one time run of
X hash iterations for each event in order to find the matching list in the hash table.
*/


/**
* @endcond 
*/

