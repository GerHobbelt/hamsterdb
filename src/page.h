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
 * @brief an object which handles a database page
 *
 */

#ifndef HAM_PAGE_H__
#define HAM_PAGE_H__

#include "internal_fwd_decl.h"

#include "endianswap.h"
#include "error.h"


#ifdef __cplusplus
extern "C" {
#endif



/*
 * indices for page lists
 *
 * each page is a node in several linked lists - via _npers._prev and
 * _npers._next. both members are arrays of pointers and can be used
 * with _npers._prev[PAGE_LIST_BUCKET] etc. (or with the macros
 * defined below).
 */

/* a bucket in the hash table of the cache manager */
#define PAGE_LIST_BUCKET           0
/* a node in the linked list of a transaction */
#define PAGE_LIST_TXN              1
/* list of all cached pages */
#define PAGE_LIST_CACHED           2
/* array limit */
#define MAX_PAGE_LISTS             3


#include <ham/packstart.h>

/**
 * The page header which is persisted on disc
 *
 * This structure definition is present outside of @ref ham_page_t scope to allow
 * compile-time OFFSETOF macros to correctly judge the size, depending
 * on platform and compiler settings.
 */
typedef HAM_PACK_0 union HAM_PACK_1 ham_perm_page_union_t
{
    /**
     * this header is only available if the (non-persistent) flag
     * NPERS_NO_HEADER is not set!
     *
     * all blob-areas in the file do not have such a header, if they
     * span page-boundaries
     *
     * @note
     * if this structure is changed, db_get_usable_pagesize has
     * to be changed as well!
     */
    HAM_PACK_0 struct HAM_PACK_1 page_union_header_t {
        /**
         * flags of this page - flags are always the first member
         * of every page - regardless of the backend or purpose - unless
         * this page is a continuation page for an ELBLOB (Extremely Large BLOB),
         * i.e. a BLOB spanning multiple pages. In that case any page but
         * the first one will only contain BLOB content and no header at all.
         *
         * Currently only used for the page type and the transaction page list:
		 * in the latter case, the flag is set in each page which sits at the
		 * start of a collision chain.
         *
         * @sa page_type_codes
         */
        ham_u32_t _flags;

        /**
         * some reserved bytes
         */
        ham_u32_t _reserved4;
        ham_u32_t _reserved5;

        /*
        WARNING WARNING WARNING WARNING WARNING WARNING

        ALIGNMENT ISSUE FOR DEC ALPHA CPU:

        64-bit int access requires 8-byte alignment: this applies to the payload data for several types of payload, inclusing B+-tree index nodes which store keys, which contain 64-bit RIDs, where all the supporting structures assume 64-bit alignment of the B+-tree node, which is our payload.
        */
#if defined(FIX_PEDANTIC_64BIT_ALIGNMENT_REQUIREMENT)
        ham_u32_t _reserved6;
#endif

        /**
         * this is just a blob - the backend (hashdb, btree etc)
         * will use it appropriately
         */
        ham_pers_multitype_t _payload[1];
    } HAM_PACK_2 _s;

    /*
     * a char pointer
     */
    ham_u8_t _p[1];

} HAM_PACK_2 ham_perm_page_union_t;

#include <ham/packstop.h>


/**
* get the size of the persistent header of a page
*
* equals the size of struct ham_perm_page_union_t, without the payload byte
*
* @note
* this is not equal to sizeof(struct ham_perm_page_union_t)-1, because of
* padding (i.e. on gcc 4.1/64bit the size would be 15 bytes)
*/
#define page_get_persistent_header_size()   OFFSETOF(ham_perm_page_union_t, _s._payload) /*(sizeof(ham_u32_t)*3)*/ /* 12 */


/**
 * the page structure
 */
struct ham_page_t
{
    /**
     * the header is non-persistent and NOT written to disk.
     * it's caching some run-time values which
     * we don't want to recalculate whenever we need them.
     */
    struct
    {
        /** address of this page */
        ham_offset_t _self;

        /** the allocator */
        struct mem_allocator_t *_alloc;

        /**
        reference to the database object; will be set for backend
        index pages, every time when such a page is allocated or
        fetched.

        This member is used as a fast way to reference the database
        backend mathods given an index page, e.g. when flushing/nuking page
        statistics at transaction abort (see @ref txn_abort()).
        */
        ham_db_t *_owner;

        /** the device of this page */
        struct ham_device_t *_device;

        /** non-persistent flags */
        ham_u32_t _flags;

        /**
         * cache counter - used by the cache module
         *
         * The higher the counter, the more 'recent' this page is
         * believed to be; when searching for pages to re-use, the empty
         * page with the lowest 'adjusted' counter value is re-purposed. Each valid
         * page use bumps up the page counter by a certain amount, up to
         * a page-type specific upper bound.
         *
         * The counter value is 'adjusted' by the hit frequency of the cached page
         * thus preferring an often accessed page over a page which was accessed only
         * once or twice, even when that latter page is more recent. This is done to
         * reduce cache thrashing when large numbers of key/record tuples are inserted
         * or erased from the database.
         *
         * See also @ref cache_put_page()
         * and its invocations in the code. @ref page_new() initialized
         * the counter for each new page.
         */
        ham_s32_t _cache_cntr;

        /**
        * cache hit count - used by the cache module
        *
        * The higher the hit count, the more 'important' this page is
        * believed to be; when searching for pages to re-use, the empty
        * page with the lowest importance/age is re-purposed. Each valid
        * page use bumps up the page counter by a certain amount, up to
        * a page-type specific upper bound.
        *
        * See also @ref cache_put_page()
        * and its invocations in the code. @ref page_new() initialized
        * the counter for each new page.
        */
        ham_s32_t _cache_hit_freq;

        /** reference-counter counts the number of transactions, which
         * access this page
         */
        ham_u32_t _refcount;

        /** the transaction Id which dirtied the page */
        ham_txn_id_t _dirty_txn;

#if defined(HAM_OS_WIN32) || defined(HAM_OS_WIN64)
        /** handle for win32 mmap */
        HANDLE _win32mmap;
#endif
        /** pointer to the 'raw' page buffer. ONLY TO BE USED by the DEVICE code! */
        //ham_u8_t *_raw_pagedata;

        /** linked lists of pages - see comments above */
        ham_page_t *_prev[MAX_PAGE_LISTS];
        ham_page_t *_next[MAX_PAGE_LISTS];

        /** linked list of all cursors which point to that page */
        ham_cursor_t *_cursors;

        /** the lsn of the last BEFORE-image, that was written to the log */
        ham_offset_t _before_img_lsn;

        /** the id of the transaction which allocated the image */
        ham_txn_id_t _alloc_txn_id;

        /**
        Exponential Moving Average (http://en.wikipedia.org/wiki/Moving_average)
        using fixed-point arithmetic. This moving average tracks the latest
        insert position within the page; future split operations get their split point
        accordingly.

        Note that the average tracks the relative position within the page, where the
        midpoint equals he zero point (0.0). This way any off-middle insertion will
        pull the EMA in that direction, which, as the EMA will always be lagging slightly
        behind and tending to the center, will deliver us a almost-ready-to-use number for
        the next split point hint.

        'Relative position' means here that the position range of the entire node is
        considered to be [-1.0 .. +1.0], i.e. the inclusive range from -1 to +1. Hence the
        EMA is a per-unage symbolizing the perunile amount of deviation from the center
        with regards to insert operations.

        The additional @ref chi2_insert_position_EMA tracks the estimated reliability of this
        average, i.e. how accurate it does track the insert position.

        To ease matters of conversion from EMA to split position hint and given the fact that
        hamsterdb supports up to 2^16-1 (= 65535 = 0xFFFF) keys, this -1..+1 range is projected
        onto the symetrical edge-exclusive range <-2^15..+2^15> i.e. the full 16-bit integer range except the
        -2^15 number.

        The fixed point is positioned between bits 15 and 16.
        */
        ham_s32_t insert_position_EMA;

        /**
        This number tracks the estimated reliability of the @ref insert_position_EMA
        number, i.e. how accurate it does track the insert position.

        This number tracks the EMA of the chi-square similar square of the distance between
        latest insert position and the tracking EMA, i.e. it represents the absolute of
        the deviation between actual and estimate. Naturally large deviations are to be
        treated as a sign that the estimate isn't all that 'trustworthy'.

        Depending on the magnitude of the tracked chi2 deviation like that, the split position
        hint 'strength' can be reduced by having the hinter reduce the 'force' of the tracking EMA
        in estimating where the optimal split point has to be.

        As hamsterdb accepts at most 2^16-1 (65535) keys in a node, the maximum deviation
        distance would equal that number, squared. As we subtract the EMA from the
        actual position to derive the distance, the worst case value we can get there would be
        MINUS 65535, while the highest positive number would be +65535, i.e. the distance number
        will consume 17 bits. When squared such a number would loose the sign, so the result
        would still fit in 32 bits. However for this to succeed the calculus must be performed
        using UNSIGNED integer arithmetic. See these example squarings:

        (printf("%d/%u/%08X/%08X --> %d/%u/%08X/%08X"))
        $ ./square -46340
        square: -46340/4294920956/FFFF4AFC/FFFF4AFC --> 2147395600/2147395600/7FFEA810/7FFEA810
        $ ./square -46341
        square: -46341/4294920955/FFFF4AFB/FFFF4AFB --> -2147479015/2147488281/80001219/80001219
        $ ./square -65535
        square: -65535/4294901761/FFFF0001/FFFF0001 --> -131071/4294836225/FFFE0001/FFFE0001

        The chi2 number hence fits in a unsigned 32-bit integer.
        */
        ham_u32_t chi2_insert_position_EMA;

    } _npers;

    /**
     * from here on everything will be written to disk
     */
    ham_perm_page_union_t *_pers;
};

/*
 * the size of struct ham_perm_page_union_t, without the payload byte
 *
 * !!
 * this is not equal to sizeof(struct ham_perm_page_union_t)-1, because of
 * padding (i.e. on gcc 4.1, 64bit the size would be 15 bytes)
 *
 * (defined in db.h)
 */
//#define page_get_persistent_header_size()   (OFFSETOF(ham_perm_page_union_t, _s._payload) /*(sizeof(ham_u32_t)*3)*/ )

/**
 * get the address of this page
 */
#define page_get_self(page)          ((page)->_npers._self)

/**
 * set the address of this page
 */
#define page_set_self(page, a)       (page)->_npers._self=(a)

/**
 * get the database object which 0wnz this page
 */
#define page_get_owner(page)         ((page)->_npers._owner)

/**
 * set the database object which 0wnz this page
 */
#define page_set_owner(page, db)     (page)->_npers._owner=(db)


#if defined(HAM_DEBUG) && (HAM_LEAN_AND_MEAN_FOR_PROFILING_LEVEL < 1)

static __inline ham_bool_t
page_is_in_list4dbg(ham_page_t *p, int which)
{
    return (p->_npers._next[which] || p->_npers._prev[which]);
}

static __inline void
page_validate_page(ham_page_t *p)
{
    /*
     * not allowed: in transaction, but not referenced
     */
    if (page_is_in_list4dbg(head, p, PAGE_LIST_TXN))
        ham_assert(page_get_refcount(p)>0,
            ("in txn, but refcount is zero"));
}

#else

#define page_validate_page(p)     (void)0

#endif

/**
 * get the previous page of a linked list
 */
static __inline ham_page_t *
page_get_previous(ham_page_t *page, int which)
{
    ham_page_t *p=page->_npers._prev[which];
    page_validate_page(page);
    if (p)
        page_validate_page(p);
    return (p);
}

/**
 * set the previous page of a linked list
 */
static __inline void
page_set_previous(ham_page_t *page, int which, ham_page_t *other)
{
    page->_npers._prev[which]=other;
    page_validate_page(page);
    if (other)
        page_validate_page(other);
}

/**
 * get the next page of a linked list
 */
static __inline ham_page_t *
page_get_next(ham_page_t *page, int which)
{
    ham_page_t *p=page->_npers._next[which];
    page_validate_page(page);
    if (p)
        page_validate_page(p);
    return (p);
}

/**
 * set the next page of a linked list
 */
static __inline void
page_set_next(ham_page_t *page, int which, ham_page_t *other)
{
    page->_npers._next[which]=other;
    page_validate_page(page);
    if (other)
        page_validate_page(other);
}

/**
 * check if a page is in a linked list
 */
static __inline ham_bool_t
page_is_in_list(ham_page_t *head, ham_page_t *page, int which)
{
    if (page_get_next(page, which))
        return (HAM_TRUE);
    if (page_get_previous(page, which))
        return (HAM_TRUE);
    if (head==page)
        return (HAM_TRUE);
    return (HAM_FALSE);
}


/**
 * get memory allocator
 */
#define page_get_allocator(page)             (page)->_npers._alloc

/**
 * set memory allocator
 */
#define page_set_allocator(page, a)          (page)->_npers._alloc=(a)

/**
 * get the device of this page
 */
#define page_get_device(page)                (page)->_npers._device

/**
 * set the device of this page
 */
#define page_set_device(page, d)             (page)->_npers._device=(d)

/**
 * get linked list of cursors
 */
#define page_get_cursors(page)           (page)->_npers._cursors

/**
 * set linked list of cursors
 */
#define page_set_cursors(page, c)        (page)->_npers._cursors=(c)

/**
 * get the lsn of the last BEFORE-image that was written to the log
 */
#define page_get_before_img_lsn(page)    (page)->_npers._before_img_lsn

/**
 * set the lsn of the last BEFORE-image that was written to the log
 */
#define page_set_before_img_lsn(page, l) (page)->_npers._before_img_lsn=(l)

/**
 * get the id of the txn which allocated this page
 */
#define page_get_alloc_txn_id(page)      (page)->_npers._alloc_txn_id

/**
 * set the id of the txn which allocated this page
 */
#define page_set_alloc_txn_id(page, id)  (page)->_npers._alloc_txn_id=(id)

/**
 * get persistent page flags
 */
#define page_get_pers_flags(page)        (ham_db2h32((page)->_pers->_s._flags))

/**
 * set persistent page flags @a or_mask while keeping the existing flags in @a and_mask intact
 */
#define page_set_pers_flags(page, or_mask, and_mask)                        \
    (page)->_pers->_s._flags=(ham_h2db32(or_mask)                           \
        | (page_get_pers_flags(page) & (and_mask)))

/**
 * get non-persistent page flags
 */
#define page_get_npers_flags(page)       (page)->_npers._flags

/**
 * set non-persistent page flags
 */
#define page_set_npers_flags(page, f)    (page)->_npers._flags=(f)

/**
* set non-persistent page flags by mixing in a given flag (bitwise OR)
*/
#define page_add_npers_flags(page, f)    (page)->_npers._flags |= (f)

/**
* unset non-persistent page flags by removing a given flag (bitwise AND)
*/
#define page_remove_npers_flags(page, f)    (page)->_npers._flags &= ~(f)


/**
 * get the cache counter
 */
#define page_get_cache_cntr(page)        (page)->_npers._cache_cntr

/**
 * set the cache counter
 */
#define page_set_cache_cntr(page, c)     (page)->_npers._cache_cntr=(c)

/**
* get the cache hit frequency
*/
#define page_get_cache_hit_freq(page)        (page)->_npers._cache_hit_freq

/**
* set the cache hit frequency
*/
#define page_set_cache_hit_freq(page, c)     (page)->_npers._cache_hit_freq=(c)

/**
* increment the cache hit frequency by one
*/
#define page_increment_cache_hit_freq(page)  (page)->_npers._cache_hit_freq++


/** page->_pers was allocated with malloc, not mmap */
#define PAGE_NPERS_MALLOC            1
/*
 * page is dirty - unused
//#define PAGE_NPERS_DIRTY             2
 */
/** page is in use */
//#define PAGE_NPERS_INUSE             4
/** page will be deleted when committed */
#define PAGE_NPERS_DELETE_PENDING   16
/** page has no header */
#define PAGE_NPERS_NO_HEADER        32

/**
 * get the txn-id of the transaction which dirtied the page
 */
#define page_get_dirty_txn(page)            ((page)->_npers._dirty_txn)

/**
 * set the txn-id of the transaction which dirtied the page
 */
#define page_set_dirty_txn(page, id)        (page)->_npers._dirty_txn=(id)

/**
 * is this page dirty?
 */
#define page_is_dirty(page)      (page_get_dirty_txn(page)!=0)

/**
 * mark the page dirty by the current transaction (if there's no transaction,
 * just set a dummy-value)
 */
#define PAGE_DUMMY_TXN_ID        1

#define page_set_dirty(page, env)                                           \
    page_set_dirty_txn(page, ((env) && env_get_txn(env)                     \
            ? txn_get_id(env_get_txn(env))                                  \
            : PAGE_DUMMY_TXN_ID))

/**
 * page is no longer dirty
 */
#define page_set_undirty(page)   page_set_dirty_txn(page, 0)

/**
 * get the reference counter
 */
#define page_get_refcount(page) (page)->_npers._refcount

/**
 * increment the reference counter
 */
#define page_add_ref(page)      ++((page)->_npers._refcount)

/**
 * decrement the reference counter
 */
#define page_release_ref(page)  do { ham_assert(page_get_refcount(page)!=0, \
                                     ("decrementing empty refcounter"));    \
                                     --(page)->_npers._refcount; } while (0)

#if defined(HAM_OS_WIN32) || defined(HAM_OS_WIN64)
/**
 * win32: get a pointer to the mmap handle
 */
#   define page_get_mmap_handle_ptr(p)      &((p)->_npers._win32mmap)
#else
#   define page_get_mmap_handle_ptr(p)      0
#endif

/**
 * set the RAW pagedata reference
 */
//#define page_set_raw_pagedata(page, ref)   (page)->_raw_pagedata=(ref)

/**
 * get the RAW pagedata reference
 */
//#define page_get_raw_pagedata(page)      (page)->_raw_pagedata

/**
 * set the page-type
 */
#define page_set_pers_type(page, t)   page_set_pers_flags(page, t, ~PAGE_TYPE_MASK)

/**
 * get the page-type
 */
#define page_get_pers_type(page)      (page_get_pers_flags(page) & PAGE_TYPE_MASK)

/**
 * @defgroup page_type_codes valid page types
 * @{
 * Each database page is tagged with a type code; these are all
 * known/supported page type codes.
 *
 * @note When ELBLOBs (Extremely Large BLOBs) are stored in the database,
 * that is BLOBs which span multiple pages apiece, only their initial page
 * will have a valid type code; subsequent pages of the ELBLOB will store
 * the data as-is, so as to provide one continuous storage space per ELBLOB.
 *
 * @sa ham_perm_page_union_t::page_union_header_t::_flags
 */

/** unidentified db page type */
#define PAGE_TYPE_UNKNOWN        0x00000000

/** the db header page: this is the very first page in the database/environment */
#define PAGE_TYPE_HEADER         0x10000000

/** the db B+tree root page */
#define PAGE_TYPE_B_ROOT         0x20000000

/** a B+tree node page, i.e. a page which is part of the database index */
#define PAGE_TYPE_B_INDEX        0x30000000

/** a freelist management page */
#define PAGE_TYPE_FREELIST       0x40000000

/** a page which stores (the front part of) an unspecified BLOB. Actual record BLOBs are stored
 * as @ref PAGE_TYPE_RECDATASPACE page/page sets. This blob type is reserved for special BLOB-like
 * page sets which have a specific role @em outside the regular database usage domain.
 *
 * @sa ham_alloc_dedicated_storage_space
 * @sa ham_release_dedicated_storage_space
 * @sa ham_fetch_dedicated_storage_space
 * @sa ham_flush_dedicated_storage_space
 */
#define PAGE_TYPE_BLOB           0x50000000

/** a page which stores (the front part of) a duplicate key table, among other things. */
#define PAGE_TYPE_DUPETABLESPACE 0x60000000
/** a page which stores extended keys, among other things. */
#define PAGE_TYPE_EXTKEYSPACE    0x70000000
/** a page which stores record data, among other things. */
#define PAGE_TYPE_RECDATASPACE   0x80000000

/** a hash table key storage overflow page, i.e. a page which is part of the database index */
#define PAGE_TYPE_H_OVERFLOW     0x90000000

/**
the mask which can be used to extract the page type code from the persisted page flags.

@sa page_union_header_t::_flags
*/
#define PAGE_TYPE_MASK           0xF0000000

/**
 * @}
 */

/**
 * get pointer to persistent payload (after the header!)
 */
#define page_get_payload(page)           (page)->_pers->_s._payload

/**
 * get pointer to persistent payload (including the header!)
 */
#define page_get_raw_payload(page)       (page)->_pers->_p

/**
 * set pointer to persistent data
 */
#define page_set_pers(page, p)           (page)->_pers=(p)

/**
 * get pointer to persistent data
 */
#define page_get_pers(page)              (page)->_pers

/**
 * linked list functions: insert the page at the beginning of a list
 *
 * @remark returns the new head of the list
 */
static __inline ham_page_t *
page_list_insert(ham_page_t *head, int which, ham_page_t *page)
{
    ham_page_t *p;

    if (!head)
	{
	    page_set_next(page, which, 0);
	    page_set_previous(page, which, page);
        return (page);
	}
	p = page_get_previous(head, which);
    page_set_previous(page, which, p);

    page_set_next(page, which, head);
    page_set_previous(head, which, page);
    return (page);
}


/**
 * linked list functions: remove the page from a list
 *
 * @remark returns the new head of the list
 */
static __inline ham_page_t *
page_list_remove(ham_page_t *head, int which, ham_page_t *page)
{
    ham_page_t *n, *p;

    if (page == head)
	{
        p = page_get_previous(page, which);
        n = page_get_next(page, which);
        if (n)
		{
            page_set_previous(n, which, p);
            //page_set_next(p, which, n);
		}
		else
		{
            ham_assert(p == head, (0));
		}
		ham_assert(page_get_next(p, which) == NULL, (0));

        page_set_next(page, which, 0);
        page_set_previous(page, which, 0);
        return (n);
    }
	else
	{
		n = page_get_next(page, which);
		p = page_get_previous(page, which);
		page_set_next(p, which, n);
		if (n)
		{
			page_set_previous(n, which, p);
		}
		else
		{
			ham_assert(page_get_previous(head, which) == page, (0));
			page_set_previous(head, which, p);
		}
		page_set_next(page, which, 0);
		page_set_previous(page, which, 0);
		return (head);
	}
}

/**
 * add a cursor to this page
 */
extern void
page_add_cursor(ham_page_t *page, ham_cursor_t *cursor);

/**
 * remove a cursor from this page
 */
extern void
page_remove_cursor(ham_page_t *page, ham_cursor_t *cursor);

/**
 * create a new page structure
 *
 * @return a pointer to a new @ref ham_page_t instance.
 *
 * @return NULL when an error occurred. The error is
 *         implied to be @ref HAM_OUT_OF_MEMORY;
 */
extern ham_page_t *
page_new(ham_env_t *env);

/**
 * delete a page structure
 */
extern void
page_delete(ham_page_t *page);

/**
 * allocate a new page from the device
 */
extern ham_status_t
page_alloc(ham_page_t *page, ham_size_t size, dev_alloc_request_info_ex_t *extra_dev_alloc_info);

/**
 * fetch a page from the device
 */
extern ham_status_t
page_fetch(ham_page_t *page, ham_size_t size);

/**
 * write a page to the device
 */
extern ham_status_t
page_flush(ham_page_t *page);

/**
 * free a page
 */
extern ham_status_t
page_free(ham_page_t *page);




#ifdef __cplusplus
} // extern "C" {
#endif

#endif /* HAM_PAGE_H__ */

/**
* @endcond
*/

