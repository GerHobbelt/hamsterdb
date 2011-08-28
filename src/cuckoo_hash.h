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
 * @brief the cuckoo_hash-backend
 *
 */

#ifndef HAM_CUCKOO_HASH_H__
#define HAM_CUCKOO_HASH_H__

#include "internal_fwd_decl.h"

#include "endianswap.h"

#include "backend.h"
#include "cuckoo_hash_cursor.h"
#include "keys.h"


#ifdef __cplusplus
extern "C" {
#endif 



/**
* defines the maximum number of keys per node
*/
#define MAX_KEYS_PER_NODE               0xFFFFU /* max(ham_u16_t) */



#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif


/**
configuration flags
*/

/**
The hash table stores the master hash with each key; for larger keys (or when 'extended keys' are
supported) this will speed up the key comparisons, thus speeding up the find/insert/erase 
operations.
*/
#define HAM_INDEX_STORES_MASTERHASHES       0x1000


/**
The 'k' in k-ary cuckoo hashing. Must be an integer >= 2 and <= 4 or a multiple of 4
*/
#define CUCKOO_K_ARITY          2

/**
The bucket size for bucketized cuckoo hashing. Must be an integer >= 1
*/
#define CUCKOO_BUCKET_SIZE      1



/**
The in-memory structure containing the various hashes used by
expansible extended k-ary bucketized cuckoo hashing.
*/
struct ham_cuckoo_hash_t
{
    ham_offset_t page;
    ham_u16_t slot[CUCKOO_K_ARITY];
};


#include <ham/packstart.h>

/**
The persisted structure containing the various hashes used by
expansible extended k-ary bucketized cuckoo hashing.
*/
typedef HAM_PACK_0 union HAM_PACK_1 ham_pers_cuckoo_hash_t
{
    ham_u64_t          hash64[1+MAX(4,CUCKOO_K_ARITY)/4];
    ham_u32_t          hash32[2+MAX(4,CUCKOO_K_ARITY)/2];
    HAM_PACK_0 struct HAM_PACK_1 
    {
        ham_u16_t flags;
        ham_u16_t page_addr[3];
        ham_u16_t slot_pos[MAX(4,CUCKOO_K_ARITY)];
    } HAM_PACK_2 combo;

} HAM_PACK_2 ham_pers_cuckoo_hash_t;

#include <ham/packstop.h>






/**
* An in-memory hash table homepage histogram info element (a single 'bar' in the histogram).
*/
typedef struct hashtable_segment_histogram_bar_t
{
    /**
    * the page index
    */
    ham_offset_t _page_index;

    /**
    * The number of keys stored in that page (plus the keys stored in the overflow collective for this page)
    */
    ham_offset_t _total_key_count;

} hashtable_segment_histogram_bar_t;


/**
* The in-memory hash table homepage histogram info.
*
* This does not include the data to aid a fast calculation of page and slot index for any key.
*/
typedef struct hashtable_segment_histogram_t
{
    /**
    * Flags.
    */
    ham_u16_t _flags;

    /**
     number of entries in the histogram
    */
    ham_u16_t _count;

    /**
    * the entries.
    */
    hashtable_segment_histogram_bar_t _entries[1];

} hashtable_segment_histogram_t;


/**
* The info for one hash table homepage segment in-memory.
*
* It records where the segment is located and its size in pages. It also records the hash histogram
* which is used for calculating the page index for any key.
*/
typedef struct hashtable_homepage_segment_t
{
    /**
    * Flags for this hash table homepage segment.
    */
    ham_u16_t _flags;

    /**
    maximum number of bars in each histogram
    */
    ham_u16_t _max_hist_size;

    /**
    * location of the segment
    */
    ham_offset_t _rid;

    /**
    number of pages in the segment
    */
    ham_offset_t _size;

    /**
    The histogram for this segment.

    @warning Note that the histograms have a dynamic size, depending on various
    factors, hence the histogram addresses should be calculated at run-time.
    */
    hashtable_segment_histogram_t *_hist_ref;

} hashtable_homepage_segment_t;


/**
* The hash table root node in-memory.
*/
typedef struct hashtable_root_node_t
{
    /**
    * Flags for this hash table.
    */
    ham_u16_t _flags;

    /**
    The expansion ratio numerator; must be larger than the denominator; is twice the
    denominator when this implementation acts as a linear hash instrad of a spiral hash 
    dynamic index.
    */
    ham_u8_t _expansion_ratio_numerator;

    /**
    The expansion ratio denominator; must be a power of 2.
    */
    ham_u8_t _expansion_ratio_denominator;

    /**
    Current expansion level of this hash table.
    */
    ham_u16_t _level;

	ham_u16_t _alignment_padding_dummy1;

    /**
    Position of the split point in the first segment.

    0 means a new expansion cycle will be starting.
    */
    ham_offset_t _split_point;

    /**
    (Approximate) number of keys stored in the hash table.
    */
    ham_offset_t _total_key_count;

    /**
    * location, size and addressing info for both segments.
    */
    hashtable_homepage_segment_t *_segments[2];

    /**
    And the reference to the third histogram which is tracking the current
    state of affairs so that the next expansion cycle can adjust its key 
    distribution accordingly.
    */
    hashtable_segment_histogram_t *_next_hist_ref;

} hashtable_root_node_t;





#include <ham/packstart.h>

/**
* A persisted hash table homepage histogram element.
*/
typedef HAM_PACK_0 struct HAM_PACK_1 hashtable_pers_segment_histogram_element_t
{
    /**
    * Index of the page in the segment
    */
    ham_pers_size64_t _page_index;

    /**
    * The number of keys stored in that page (plus the keys stored in the overflow collective for this page)
    */
    ham_pers_size64_t _total_key_count;

} HAM_PACK_2 hashtable_pers_segment_histogram_element_t;

#include <ham/packstop.h>



#include <ham/packstart.h>

/**
* The persisted hash table homepage histogram info.
*/
typedef HAM_PACK_0 struct HAM_PACK_1 hashtable_pers_segment_histogram_t
{
    /**
    * Flags.
    */
    ham_u16_t _flags;

    /**
    number of entries in the histogram
    */
    ham_u16_t _count;

    ham_u32_t _reserved21;

    /**
    * The entries of this histogram.
    */
    hashtable_pers_segment_histogram_element_t _entries[1];

} HAM_PACK_2 hashtable_pers_segment_histogram_t;

#include <ham/packstop.h>





#include <ham/packstart.h>

/**
* The info for one hash table homepage segment.
*
* It records where the segment is located and its size in pages. It also records the hash histogram
* which is used for calculating the page index for any key.
*/
typedef HAM_PACK_0 struct HAM_PACK_1 hashtable_pers_homepage_segment_t
{
    /**
    * Flags for this hash table homepage segment.
    */
    ham_u16_t _flags;

    ham_u16_t _reserved21;

    ham_u32_t _reserved22;

    /**
    * location of the segment
    */
    ham_pers_rid_t _rid;

    /**
     number of pages in the segment
    */
    ham_pers_size64_t _size;

    /**
    The histogram for this segment.

    @warning Note that the histograms have a dynamic size, depending on various
    factors, hence the histogram addresses should be calculated at run-time.
    */
    hashtable_pers_segment_histogram_t _hist[1];

} HAM_PACK_2 hashtable_pers_homepage_segment_t;

#include <ham/packstop.h>





#include <ham/packstart.h>

/**
* The hash table root node.
*
* It records where both homepage segments are located plus all mandatory info required
* to address the proper homepage given any key.
* The first segment is the the hash table before expansion, while the second segment
* is the table after the current expansion. It also records the third hash histogram
* which is to be used for the next (future) expansion.
*
* Note that a table size <em>reduction</em> is treated as just another expansion,
* but now a 'expansion' which has an expansion ratio less than one(1).
*/
typedef HAM_PACK_0 struct HAM_PACK_1 hashtable_pers_root_node_t
{
    /**
    * Flags for this hash table.
    */
    ham_u16_t _flags;

    /**
    The expansion ratio numerator; must be larger than the denominator; is twice the
    denominator when this implementation acts as a linear hash instrad of a spiral hash 
    dynamic index.
    */
    ham_u8_t _expansion_ratio_numerator;

    /**
    The expansion ratio denominator; must be a power of 2.
    */
    ham_u8_t _expansion_ratio_denominator;

    /**
    expansion level of this hash table
    */
    ham_u16_t _level;

    ham_u16_t _reserved21;

    /**
    Position of the split point in the first segment.

    0 means a new expansion cycle will be starting.
    */
    ham_pers_size64_t _split_point;

    /**
    (Approximate) number of keys stored in the hash table.

    @note Though one might believe in strict adherence and update the root node for every
    transaction, this can become a quite costly activity. As the root node only really 
    <em>must</em> be updated when another (partial) page expansion has been performed,
    we only flush the root node to persisted storage at those moments and leave it
    as-is in the RAM cache.

    Assuming a crash or other fatal failure, we can be certain the number of keys reported 
    here is only an approximation as several possible updates of the number didn't make
    it to persisted storage before the hypothetical crash. But we do not mind as here we
    knowingly choose performance over data accuracy. Hence this number can only serve for
    statistical approximations and a 
            SELECT COUNT(*) FROM TABLE
    query must therefore still scan the hash table when an exact number is required.

    Incidentally, the same reasoning applies to the histograms stored in this node: they
    are a (close) approximation of reality.

    Errors in the histograms do not accumulate as each update/erase operation will 
    update the data in a history-independent fashion, so previous errors in these numbers
    are (at least partially) gone as soon as the root node is saved to persisted storage 
    once again after such update/erase activity.
    */
    ham_pers_size64_t _total_key_count;

    /**
    * location, size and addressing info for both segments, plus the third histogram.
    *
    * @warning Note that the histograms have a dynamic size, depending on various
    * factors, hence the segment info addresses should be calculated at run-time.
    *
    * The layout of the root node is not clearly visible from this simplified definition:
    * first both segments and their histograms are stored, followed by the third histogram
    * <em>without</em> a segment structure preceding it. In other words: the third histogram
    * immediately follows the second histogram in the persisted root node.
    *
    * The size of the histograms depends on the storage page size: the histograms are 
    * dimensioned to ensure the root node record spans more than one persistent storage page.
    */
    hashtable_pers_homepage_segment_t _segments[1];

} HAM_PACK_2 hashtable_pers_root_node_t;

#include <ham/packstop.h>






#include <ham/packstart.h>

/**
* A hash table key entry; to speed up checks we store the master hash with each key on the page.
* Yes, this increases the overhead per key, i.e. cuts down on the number of keys which can be stored
* on a page, but this way is expected to be faster anyway.
*/
HAM_PACK_0 struct HAM_PACK_1 hashed_key_t
{
    /**
    * The master hash for this key
    */
    ham_pers_cuckoo_hash_t _master_hash;

    /**
    * the key and record reference.
    */
    int_key_t _key;

} HAM_PACK_2;

#include <ham/packstop.h>

/**
* get the size of the internal key representation header when the master hash is stored with each key
*
* @sa HAM_INDEX_STORES_MASTERHASHES
*/
#define db_get_hash_key_header_size()   (OFFSETOF(hashed_key_t, _key) + db_get_int_key_header_size())






#include <ham/packstart.h>

/**
* A hash table home-page; it spans the persistent part of a ham_page_t:
*
* <pre>
* hashtable_home_node_t *ptr = (hashtable_home_node_t *)page->_u._pers.payload;
* </pre>
*/
typedef HAM_PACK_0 struct HAM_PACK_1 hashtable_pers_homepage_t
{
    /**
    * Number of hashes stored in this page.
    */
    ham_u16_t _count;

    /**
    * Flags for this hash table homepage.
    */
    ham_u16_t _flags;

    /**
    (Approximate) number of hashes stored in overflow pages for this homepage.

    The number is formatted like a floating point number, where the point is located
    past bit 0: the most significant 6 bits contain the power-of-2, while the remaining
    bits contain the mantissa, thus storing a 64-bit number in 16 bits.
    */
    ham_u16_t _overflow_count;

    ham_u16_t _reserved21;

    /**
    * address of the overflow page collective
    */
    ham_pers_rid_t _overflow_collective_rid;

    /**
    * the entries of this node.

    Depending on the configuration settings, we either store the master hash with each key
    or require a recalculation of the hash for every access.
    */
    HAM_PACK_0 union HAM_PACK_1 
    {
        hashed_key_t hash;
        int_key_t raw_key;
    } HAM_PACK_2 _entries[1];

} HAM_PACK_2 hashtable_pers_homepage_t;

#include <ham/packstop.h>





/**
* The backend structure for the extended spiral hashing backend.
*/
struct ham_spiral_hashtable_t
{
    BACKEND_DECLARATIONS(ham_spiral_hashtable_t, common_hashtable_datums_t);
};


typedef struct common_hashtable_hints_t
{
    int _reserved1;
} common_hashtable_hints_t;



/**
This structure stores all 'constant' datums related to the current Database to be passed around
the hashtable backend methods.

The intent here is to calculate these items once, upon entering a backend method (callback) and
then only taking up a single pointer pushed on the stack with each internal function call in the
backend, while enabling all these functions easy and fast access to this information.

Some items (the @ref ham_key_t and @ref ham_record_t references for instance) are stored in here
to allow the backend to pass these pieces of information to the RID-assigning methods of the underlying
device (which may employ this information for partitioning and other storage layout schemes), while
other items are just kept in here to cut down on the number of dereferences through pages, cursors, etc.
to get at them.

The prime directive for this structure is always: attempt to cut down on the run-time cost in the 
backend, either through nested or repetitive dereferences in one of more internal functions or through
reducing the stack frame construction effort as a result of pushing many function arguments on the stack
as those are passed around.
*/
struct common_hashtable_datums_t
{
    ham_spiral_hashtable_t *be;

    ham_db_t *db;

    ham_env_t *env;

    ham_device_t *dev;

    ham_key_t *in_key;

    ham_record_t *in_rec;

    /**
    * A pointer to a cursor; if this is a valid pointer, then this 
    * cursor will point to the new inserted / located (found) item.
    */
    ham_ch_cursor_t *cursor;

    ham_u32_t flags;

    ham_u16_t keysize;

    ham_u16_t maxkeys;

    ham_size_t keywidth;

    unsigned has_fast_index : 1;

	unsigned _alignment_padding_dummy1 : 31;

    ham_size_t offset_to_fastindex;

    ham_size_t offset_to_keyarr;

    common_hashtable_hints_t hints; 

    /** The ratio where a node is split: (maxkeys * ratio) */
    ham_float_t split_ratio;

    /** The ratio where a node is merged: (maxkeys * ratio) */
    ham_float_t merge_ratio;
};


/**
* defines the maximum number of keys per node
*/
//#define MAX_KEYS_PER_NODE             0xFFFFU /* max(ham_u16_t) */



/**
* "constructor" - initializes a new ham_spiral_hashtable_t object
*
* @return a pointer to a new created dynamic hash table backend 
*         instance in @a backend_ref
*
* @remark flags are from @ref ham_env_open_db() or @ref ham_env_create_db()
*/
extern ham_status_t
spiral_hashtable_create(ham_backend_t **backend_ref, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param);





#ifdef __cplusplus
} // extern "C"
#endif 

#endif /* HAM_CUCKOO_HASH_H__ */

/**
* @endcond 
*/

