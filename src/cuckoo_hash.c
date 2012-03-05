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
* @brief implementation of cuckoo hash database (index) backend
* @author Ger Hobbelt, ger@hobbelt.com

Dynamized cuckoo hash:

Extended Spiral Hashing for Expansible Files (1999, YE-IN CHANG, CHIEN-I LEE AND WANN-BAY CHANGLIAW)

Filtered Linear Hashing (1999, Ang, Tan)

and mix that with a k-ary bucketized cuckoo hash table approach instead of the single-hash
approach in linear and spiral hashing;
apply the filtering idea to spiral hashing alongside ==> extended, filtered spiral cuckoo hashing.



Things to keep in mind:

multi-threaded access: when splitting pages, the hashes are spread across multiple pages; in
the case of extended spiral hashing, due to there being _partial_ pages when
a non-integer ratio such as 3/2 is used, part of the split hashes land in an already
ACTIVE page (we don't /do/ 'partial pages', it's just that those pages have a low fill
rate equivalent to a 'partial page' until the next split 'fills it up').

Now suppose we need to rollback that split: do we need to lock those pages and rewrite
their original content during rollback?

A: no, we don't need to rewrite the old content IFF we stick to this rule: any page can
only legally store the hash range designated to it.
That means: all the new hashes which were copied to the page are INVALID by implication
as the rollback happens as those hashes sit in a hash range which lies outside
the 'legal scope' of that page following the rollback.

When an insert or delete hits such an now-invalid hash it should be quickly recognizable
as such; as the full hash (including the part used to decide on which page it
should land in) is stored as a key, this takes one hash comparison to designate the
store point equal to an 'empty slot'.

Additionally, each page must store two hash markers, as a page can, after splitting,
contain at most two 'partial pages', so only 1 or 2 page designating hash values
can point to this page; by storing them both as page parameters, the key comparison
only requires local data.

It MIGHT be feasible to /calculate/ those two page hashes on the fly, which would
preclude the need to store them in each page. After all, those hashes are known and
calculable anyway as they are part of the zero-I/O O(1) page addressing scheme of
linear/spiral hashing. As the constant 'c' is rolled back as part of the rollback,
this calculation would deliver the 1 or 2 legal table hashes for any page, so an insert
or delete can use those to detect which slots in the page are 'empty', when
'empty' is not a flag but an 'out of range' hash key value (zero at start; may be
other values too given a rolled-back page split).


The above idea can also be used to simplify table /shrinkage/ following key deletes: new
allocated pages are zeroed; partial fills are done as usual; any rollback just
means some inserts are now implicitly 'empty' slots as their hash key values are
'out of range' for the given page.

Those pages still need to be locked for write access, but they don't have to be rolled
back, thus cutting down on logging costs.
Hence a key insert operation only needs to unroll and recover the page image of the
page where the key itself was stored; page splits don't require this type of rollback.



In memory and on disc, the hash table consists of two chunks: the old chunk of pages
and the new chunk of pages where the hashes will migrate to when each old page
is split. So there's no realloc() required, just two references, one for each chunk
of pages.

How about the overflow pages? When we use separators, we still don't know the maximum
number of overflow pages. Though me /may/ guestimate the /probable/ worst case to
be N*R-1 pages where the hash table itself is N pages and the full split ratio = R
(3/2 for extended spiral, 2 for linear hashing):
assuming the worst possible key distribution, all keys will land in a single page of
the hash table, which will overflow. As the hash table is redimensioned using a split
control which ensures the hash table is large enough to store all keys at the time of
full expansion, the overflow size will be equal to the size of the fully expanded
table minus the 1 page of the hash table itself.

How do we store those keys on those overflow pages? Do we use an ordered sequential
store a la Btree or do we use hash tables again?

Given the separators approach, and ordered sequential store would be the simplest thing
to do, but search cost will not be O(1) then (though hash median guestimation can
get us near that number; given that the keys are hashes, we can calculate a number
representing 'distance' so we don't need a complete binary search then).
Overflow page splitting will be easier and faster for the ordered sequential search,
but the hash table per page approach would be faster for the insert itself as fewer
items will have to be moved around given semi-random (or at least not sequentially
incrementing) insert loads.

So... we don't use separators, or maybe we do.

Anyway, the current idea is to keep a tiny overflow zone per page cf. the 'stash' idea
for cuckoo hashing, and when that tiny 'stash' overflows, the hashes end up in a 'global
overflow store', i.e. a single store for all overflow keys.
This may not be very smart to do as it makes extracting and reinserting the overflow
keys for a particular page when it is split more time consuming as all other keys in such
a store must be skipped during the scan, so a per-page overflow chain sounds most reasonable.

However, storing the separator array in the 'home page' is not a very good idea when such
a separator array has unknown and arbitrary length, so there's another overflow scenario
there: storing the surplus of the separator chain in another page when the overflows get
too much.

However, /when/ will we ever have so many overflows that we need so many overflow pages?

Assuming an extremely skewed hash distribution, which has all the hashes land in the
top-most page, i.e. the page which will be split last, then with an initial hash table size
of N pages and a target fill ratio A before the table is fully split, there will be N*A-1
overflow pages in that particular chain.

This can be limited by forcing faster splitting when the first overflow page starts to fill up.
Incidentally, that overflow page can be the new page position after split, so that the keys are
already in place there; this saves additional copying and key reprocessing work during the
coming split.
After all, all the pages for the next full split cycle are allocated in a single bundle
to keep page address calculations simple, so we can use that fact to use those new pages
as overflow pages initially. That's one-point-x overflow pages per page readily available:
given the expansion ratio R, the available overflow space per page is R pages.

When the overflow requirements grow beyond that, we may decide two ways: add additional overflow
pages and track their fill rates, maybe using separators for O(2) addressability throughout,
or force a multi-page split and start the next split cycle, thus causing a higher cost for
that particular last insert action (or set of insert actions, when we panic gradually using
a high-water mark for starting this type of escalation).

Both ways the hash functions used should be adapted in some way to reduce these extreme peaks in the
key distribution.
Which leads us, again, to another type of histogram (which should be persisted) to help us
analyze hash key distribution.
But what can we do when we have this histogram? How can we use that data to transform the hash
values into a seemingly 'flatter' distribution?

First of all, the hash function (and all its underlying mechanisms, such as reshuffling and
transformations) can only be changed at the time of a completed split, when the old hash function
and page space is discarded and a new split cycle will start.

Hence our 'response time' to effect this kind of adjustments is one entire page split cycle; this
may be a long time to come when the table is large.

So it is imperative to allow for a larger overflow area than just 1-point-x pages if we don't want
to hasten the page split work and thus increase the load per insert significantly.

Second, how would such a transform look? Local 'zooming' of the hash value range comes to mind,
but given integer hashes, the only effect that would cause is introducing gaps in the zoomed area
and a higher chance at collisions for near hash values in the 'shrunk' hash value zones.

Besides the histogram would ideally keep one data bar per hash table page, but that would be a
variable size, so we must lump page info in the histogram for large tables when we wish to stick
with the ease of persistence and code a fixed width histogram gives us.
The alternative solution here is to use more hash bits for page address calculations in the 'zoomed'
zone(s), which exhibit(ed) relative large overflow page demands, while page groups which require
fewer overflow pages (or none at all) can do will fewer bits of the hash to calculate their
page addresses.

In other words: we partition the hash table in G groups, one group per histogram bar, and determine
the way the hash page address is to be calculated per group.

So hash index calculus is as follows:

key --> h(key, c, s) --> G=fG(h, c, s) --> page = fP(h, G, c, s) --> slot = fS(h, page, G, c, s)

where fG(), fP() and fS() are functions which extract suitable, independent, index values from the
hash h and c is the current expansion level and s the current expansion state (how far the current
split has progressed).

Next, when a slot in the addressed page does not carry the searched key, cuckoo hashing can be applied
on a per-page or per-group basis: per-page has the benefit of reduced disc I/O, but with very skewed
hash distributions the keys will be stuck in the space of this single page. (I am willing to
live with that limitation for now.)

Next, when cuckoo hashing delivers a 'not found' verdict and the page info indicates overflow pages
are in use, then the overflow page address can be calculated as follows:

first calculate G, page and slot for the next level of expansion which is currently underway: the
first overflow page is identical to the space where the keys will end up after the ongoing expansion.
(See above for this speed optimization idea: since disc page cost is relatively cheap, we allocate
entire chunks of empty pages at once for each expansion cycle: this in turn keeps our page address
calculation efforts low, plus reduces the cost of splitting overflowing pages as the overflow already
took care of that part of the split action anyhow, at least for the first overflow page.)

This next page/slot access is the second disc access, so we're O(2) by now. If that one /again/
indicates further overflow pages are used, then those must have to be checked as well; we can use
a limited-sized separator table for those to directly address those overflow pages; we can also
w.h.p. say that this situation will not happen for reasonable hash distributions, i.e. for properly
designed hash functions, so keeping them in a global, partitioned store should suffice.
A B-tree comes to mind here.


Idea for the histrogram to make it dynamically resizable, in a way: as each hash table page can keep
track of its own fill rate anyway, we already have the basics for the full size histogram
available to us, though in 'ditributed' form.

What is important to answer is which pages are carrying the bulk of the hashes, so we really are
only interested in having the top X pages immidiately available to us without any disc access cost.
So all the histogram has to do is, say, keep the 16 fullest pages listed, giving us a good shot
at flattening those pages at least by spreading their hash values across multiple pages during the
next split cycle. Hence we only need to keep three histograms around: the one used with the
address calculation for the 'original' table, the one used for the /current/ split cycle page addressing
and the one which is continually updated to assist in the page calculus for the /next/ split cycle.

Furthermore, as we are only interested in pages which overflow, we don't mind about home pages
getting full. Only once an insert is going to access overflow space will we have to take a closer
look. As such overflow storage activity accesses the new split pages only, we have their fill numbers
readily available to us at insert time (and at page split time, which is just a specialized
batched (re)insert operation), so we can compare those numbers with our current top X pages and see
whether we need to update an entry or replace an entry in our histogram with the current page index
and count, thus keeping a real-time top X around. Once the split cycle has completed, the third
histogram is shifted to second place (all three histograms shift one place left, thus discarding
the oldest histogram as its related are discarded as well) and a new tracking histogram is
initialized at third place and the work starts all over again, producing a (slowly) adaptive scheme
which compensates, at least partially, for skewed hash distributions in a spiral/linear hashing
environment.

This compensation is done by 'expanding' pages which see lots of overflows. We do /not/ 'bunch up'
other pages as that may cause additional collisions there. Hence the real expansion factor is slightly
larger than the default R as for each of the pages in the top X more than R pages will be addressed
after the split, resulting in a larger table. The total table size can be determined at the start of
the split cycle because the top X histogram and every other parameter is known by then and fixed.

The 'old' pages are addressed using the histogram which led to their placement, the new pages are
addressed using the newer (second) histogram, and there is no need to keep a 'history trace' of these
as each stand on their own: when the skewed distribution remains as is, then the second and first
histograms will resemble each other quite closely. It may happen that pages which were split across
a larger set of pages in a previous round now land on relatively fewer pages again as the peak of
the skewed distribution is narrowed down as the table grows during multiple full split rounds. This
is not harmful in any way: the most demanding pages are always detected during the split cycle and
compensated for in the next.

The only risk here is with very dynamic tables where inserts and erasures are mixed 1:1 so no table
growth is required while the hash distribution changes skew position due to a very bad hash function
and changing key input distribution during the lifetime of the table: only after two expansion rounds
will the hash table be able to (mostly) compensate for the skew that existed.


How large must be our master hash?

Since the 'group' idea has just left the building, we have a page address calculation and a slot
position. Using cuckoo hashing, that's two slot positions, or k slots for k-ary cuckoo.
We may want extra hash bits for working with the overflow addressing, but I think one extra bit
is enough as the first overflow 'page' is an expanded page of size R:1, where R<=2
(R=2 is linear hashing); further overflow handling work needs other things done anyhow.

At an abs. max. of 64K keys per page and k cuckoo hashes, that requires k*16 bits plus 64-16 = 48 bits
for page addressing of any kind (including the overflow addressing) when we want to be able to
store 2^64 keys in the table. For k=4 64+48=112 bits of master hash should do it.

Using two 64-bit words of overhead, this leaves us with 16 bits for flags and miscellaneous. Add to
that a 64-bit RID for addressing the record and N bytes for the key and we have an official overhead
estimate of 3*64 bits = 24 bytes per key, which is more than the overhead for the Btree.

The alternative would be to /not/ store the master hash with each key, but that would result in
increased calculation costs as hash positions are checked in the cuckoo hash table, which is detrimental
to our performance when we allow large keys or 'extended keys' in particular as our current O(3)
disc I/O scheme would then explode to O(3+k-1) for extended key comparisons.

We don't achieve O(2) in worst worst case as the overflow area is split in two: the expanded pages
first and then the additional overflow area, using separators or a (B)tree for fast access
(O(1) vs O(logB(N)).




Write access for the root page histograms is only required for resize operations (page split/shrink)
as those are the only operations that really 'sense' and impact key distribution. Only having those
ops write to the root page stats improves concurrency as otherwise the root page would quickly become a
'hot spot'.

As a way out, any write operation /may/ write-lock the homepage and update the counters there, though
even that is not mandatory: the overflow count could be tracked in the first overflow page instead,
thus dividing statistics counts across current and future homepages (as the future homepages also
serve as the initial overflow pages for the current homepages).

To keep these numbers apart from the 'fill rate' of the homepage itself, each homepage should have
/two/ counters: one for itself and one for the 'lump sum' overflow count. However, as these numbers
are extant in different pages at different times, one counter may suffice: since we know which pages
constitute our 'current homepage set', we can detect what the /meaning/ of the counter
in each page really is. At the time of a page split operation, the overflow count can be extracted
and zeroed as, by then, that page will become part of the current homepage set. Where to leave the
overflow count then, when the page split operation itself causes overflow? That number would then
have to reside outside the potential homepage set, either in the 'second level' overflow pages/tree,
or only in the root page histogram; after all, only the largest overflow counts matter when it
comes to determining the histograms and resulting page spread / page index calculations, now and
in the future cycle.

*/

#include "internal_preparation.h"

#include "cuckoo_hash.h"



ham_size_t
hashtable_calc_maxkeys4homepage(ham_u16_t dam, ham_size_t pagesize, ham_u16_t keysize, ham_u32_t flags)
{
    ham_size_t p;
    ham_size_t k;
    ham_size_t max;

    /*
    * a hash table homepage is always P bytes long, where P is the pagesize of
    * the database.
    */
    p = pagesize;

    /* every hash table homepage has a header where we can't store entries */
    p -= OFFSETOF(hashtable_pers_homepage_t, _entries);

    /* every page has a header where we can't store entries */
    p -= page_get_persistent_header_size();

    /*
    * compute the size of a key, k.
    */
    if (flags & HAM_INDEX_STORES_MASTERHASHES)
    {
        k = keysize + db_get_hash_key_header_size();
    }
    else
    {
        k = keysize + db_get_int_key_header_size();
    }

    max = p / k;

    return max;
}






/**
* create and initialize a new spiral hashtable backend
*
* @remark this function is called after the @a ham_db_t structure
* and the file were created
*
* the @a flags are stored in the database; only transfer
* the persistent flags!
*/
static ham_status_t ch_fun_create(ham_spiral_hashtable_t *be, ham_u16_t keysize, ham_u32_t flags)
{
    ham_status_t st;
    ham_page_t *root;
    ham_size_t maxkeys_per_homepage;
    dev_alloc_request_info_ex_t info = {0};
    ham_db_t *db = be_get_db(be);
    ham_env_t *env = db_get_env(db);
    db_indexdata_t *indexdata = env_get_indexdata_ptr(env, db_get_indexdata_offset(db));

    if (be_is_active(be))
    {
        ham_trace(("backend has alread been initialized before!"));
        return HAM_ALREADY_INITIALIZED;
    }

    /*
     * calculate the maximum number of keys for this page
     *
     * prevent overflow - maxkeys only has 16 bit!
     */
    maxkeys_per_homepage = hashtable_calc_maxkeys4homepage(db_get_data_access_mode(db), env_get_pagesize(env), keysize, flags);
    if (maxkeys_per_homepage > MAX_KEYS_PER_NODE)
    {
        ham_trace(("keysize/pagesize ratio too high"));
        return HAM_INV_KEYSIZE;
    }
    else if (maxkeys_per_homepage == 0)
    {
        ham_trace(("keysize too large for the current pagesize"));
        return HAM_INV_KEYSIZE;
    }

    /*
    * allocate a new root page
    */
    info.db = db;
    info.env = env;
    info.dam = db_get_data_access_mode(db);
    info.entire_page = HAM_TRUE;
    info.space_type = PAGE_TYPE_B_ROOT;
    st = db_alloc_page(&root, PAGE_IGNORE_FREELIST, &info);
    ham_assert(st ? root == NULL : 1, (0));
    ham_assert(!st ? root != NULL : 1, (0));
    if (st)
        return st;

    /* BUGFIX: this destroyed the persisted page header too; that part is cleared inside db_alloc_page() */
    memset(page_get_payload(root), 0, env_get_pagesize(env) - page_get_persistent_header_size());
    page_set_owner(root, db);

    //btree_set_maxkeys(be, (ham_u16_t)maxkeys);
    be_set_dirty(be, HAM_TRUE);
    be_set_keysize(be, keysize);
    be_set_flags(be, flags);

    //btree_set_rootpage(be, page_get_self(root));

    index_clear_reserved(indexdata);
    //index_set_max_keys(indexdata, (ham_u16_t)maxkeys);
    index_set_keysize(indexdata, keysize);
    index_set_self(indexdata, page_get_self(root));
    index_set_flags(indexdata, flags);
    index_set_recno(indexdata, 0);
    index_clear_reserved(indexdata);
    // if (!db_is_mgt_mode_set(db_get_data_access_mode(db),
    //                      HAM_DAM_ENFORCE_PRE110_FORMAT))

    env_set_dirty(env);

    be_set_active(be, HAM_TRUE);

    return HAM_SUCCESS;
}

/**
* open and initialize a spiral hashtable backend
*
* @remark this function is called after the ham_db_structure
* was allocated and the file was opened
*/
static ham_status_t ch_fun_open(ham_spiral_hashtable_t *be, ham_u32_t flags)
{
    return HAM_INTERNAL_ERROR;
}

/**
* close the spiral hashtable backend
*
* @remark this function is called before the file is closed
*/
static ham_status_t ch_fun_close(ham_spiral_hashtable_t *be)
{
    return HAM_INTERNAL_ERROR;
}

/**
* flush the spiral hashtable backend
*
* @remark this function is called during ham_flush
*/
static ham_status_t ch_fun_flush(ham_spiral_hashtable_t *be)
{
    return HAM_INTERNAL_ERROR;
}

/**
* find a key in the index
*/
static ham_status_t ch_fun_find(ham_spiral_hashtable_t *be, ham_key_t *key, ham_record_t *record, ham_u32_t flags)
{
    return HAM_INTERNAL_ERROR;
}

/**
* insert (or update) a key in the index
*
* the spiral hashtable backend is responsible for inserting or updating the
* record. (see blob.h for blob management functions)
*/
static ham_status_t ch_fun_insert(ham_spiral_hashtable_t *be, ham_key_t *key, ham_record_t *record, ham_u32_t flags)
{
    return HAM_INTERNAL_ERROR;
}

/**
* erase a key in the index
*/
static ham_status_t ch_fun_erase(ham_spiral_hashtable_t *be, ham_key_t *key, ham_u32_t flags)
{
    return HAM_INTERNAL_ERROR;
}

/**
* iterate the whole tree and enumerate every item
*/
static ham_status_t ch_fun_enumerate(ham_spiral_hashtable_t *be, ham_enumerate_cb_t cb, void *context)
{
    return HAM_INTERNAL_ERROR;
}

/**
* verify the whole tree
*/
static ham_status_t ch_fun_check_integrity(ham_spiral_hashtable_t *be)
{
    return HAM_INTERNAL_ERROR;
}

/**
* free all allocated resources
*
* @remark this function is called after _fun_close()
*/
static ham_status_t ch_fun_delete(ham_spiral_hashtable_t *be)
{
    return HAM_INTERNAL_ERROR;
}

/**
* estimate the number of keys per page, given the keysize
*/
static ham_status_t ch_fun_calc_keycount_per_page(ham_spiral_hashtable_t *be, ham_size_t *keycount, ham_u16_t keysize)
{
    return HAM_INTERNAL_ERROR;
}

/**
Create a new cursor instance.
*/
static ham_status_t ch_fun_cursor_create(ham_spiral_hashtable_t *be, ham_db_t *db, ham_txn_t *txn, ham_u32_t flags, ham_cursor_t **cu)
{
    return HAM_INTERNAL_ERROR;
}

/**
Close (and free) all cursors related to this database table.
*/
static ham_status_t ch_fun_close_cursors(ham_spiral_hashtable_t *be, ham_u32_t flags)
{
    return HAM_INTERNAL_ERROR;
}

/**
* uncouple all cursors from a page
*
* @remark this is called whenever the page is deleted or
* becoming invalid
*/
static ham_status_t ch_fun_uncouple_all_cursors(ham_spiral_hashtable_t *be, ham_page_t *page, ham_size_t start)
{
    return HAM_INTERNAL_ERROR;
}

/**
* Remove all extended keys for the given @a page from the
* extended key cache.
*/
static ham_status_t ch_fun_free_page_extkeys(ham_spiral_hashtable_t *be, ham_page_t *page, ham_u32_t flags)
{
    return HAM_INTERNAL_ERROR;
}

/**
Nuke the given page's statistics for the given @a reason.
*/
static void ch_fun_nuke_statistics(ham_spiral_hashtable_t *be, ham_page_t *page, ham_u32_t reason)
{
}

/**
* enumerate a single page
*/
static ham_status_t ch_fun_in_node_enumerate(common_hashtable_datums_t *btdata, ham_cb_enum_data_t *cb_data, ham_enumerate_cb_t cb)
{
    return HAM_INTERNAL_ERROR;
}










ham_status_t
spiral_hashtable_create(ham_backend_t **backend_ref, ham_db_t *db, ham_u32_t flags, const ham_parameter_t *param)
{
    ham_spiral_hashtable_t *tbl;
    ham_env_t *env = db_get_env(db);

    *backend_ref = 0;

    tbl = (ham_spiral_hashtable_t *)allocator_calloc(env_get_allocator(env), sizeof(*tbl));
    if (!tbl)
    {
        return HAM_OUT_OF_MEMORY;
    }

    /* initialize the backend */
    tbl->_db=db;
    tbl->_fun_create=ch_fun_create;
    tbl->_fun_open=ch_fun_open;
    tbl->_fun_close=ch_fun_close;
    tbl->_fun_flush=ch_fun_flush;
    tbl->_fun_delete=ch_fun_delete;
    tbl->_fun_find=ch_fun_find;
    tbl->_fun_insert=ch_fun_insert;
    tbl->_fun_erase=ch_fun_erase;
    tbl->_fun_enumerate=ch_fun_enumerate;
    tbl->_fun_check_integrity=ch_fun_check_integrity;
    tbl->_fun_calc_keycount_per_page=ch_fun_calc_keycount_per_page;
    tbl->_fun_cursor_create = ch_fun_cursor_create;
    tbl->_fun_close_cursors = ch_fun_close_cursors;
    tbl->_fun_uncouple_all_cursors = ch_fun_uncouple_all_cursors;
    tbl->_fun_free_page_extkeys = ch_fun_free_page_extkeys;
    tbl->_fun_nuke_statistics = ch_fun_nuke_statistics;

    //tbl->_fun_in_node_get_key_ref = btree_in_node_get_key_ref;
    //tbl->_fun_in_node_shift_to_page = ch_fun_in_node_shift_to_page;
    tbl->_fun_in_node_enumerate = ch_fun_in_node_enumerate;
    //tbl->_fun_in_node_verify_keys = btree_in_node_verify_keys;

    *backend_ref = (ham_backend_t *)tbl;
    return HAM_SUCCESS;
}


/**
* @endcond
*/

