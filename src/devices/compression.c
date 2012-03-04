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
@brief A 'virtual device' which compresses data on a per-page basis before it is stored
       on a physical device.

Note that this device produces arbitrary length page sizes 'downwind' (write direction)
and all attached devices, virtual and physical, should be able to cope with such (while
performance MAY be impacted adversely for some).

To allow for fast addressing, a 'microdatabase' is employed. This uDB maps compressed addresses and
page widths to their uncompressed equivalents, and vice versa.

The microdatabase has a custom format to reduce overhead and provide maximum performance:
We are mapping 64-bit addresses to 64-bit offsets (addresses) through an augmented digital trie.

As addresses are assigned in semi-sequential order, we 'know' the trie will exist at the left edge
first and slowly grow towards the outer right edge. To keep trie storage consumption to a minimum,
each node is marked as either 'terminating' or not. Terminating nodes are those which do NOT contain
any valid pointers to other nodes (yet). Non-terminating nodes will contain a set of addresses,
which, depending on the current depth, either point to another trie node (making this trie node a
'branch') or list the target address/offset ('leaf node entry').

The digital trie is split on a 8-bit basis, resulting in an absolute maximum trie depth of 8 for
64-bit offsets.

The initial storage demand will be 256*64 bits = 2K bytes.

(offset/address -1 is used as a special marker. So is -2, etc., which aids in keeping the trie nodes
at a 2K alignment everywhere. Initially, only the depth=8, left-most node is created and declared
'root': the actual 'root' node is always at uDB offset 0, so, once the uDB node count needs to grow
beyond that initial 256 count, the previous 'root' can be copied to another page and the new 'root'
can point the appropriate pointer/address-offset to that node.

As the largest trie can span the 64-bit address range, where each node uses 8 bytes, the maximum number
of nodes that can be stored in this trie is
     N + ... == 2^64 / 8  ==>  N < 2^61

1
N + 1
N/C * (N + 1) + 1 * N + 1
N/C^2 * (N/C * (N + 1) + N + 1) + 1 * (N/C * (N + 1) + N) + 1

1
N + 1
(N/C + 1) * (N + 1)       N^2/C + 2/C N + 1
(N/C^2 + 1) * ...
(N/C^3 + 1) * ...
(N/C^4 + 1) * ...
(N/C^5 + 1) * ...
(N/C^6 + 1) * ...
(N/C^7 + 1) * ...         N^8/C^7 + 8/C^6 N^7 + ...

which means that, as we are mapping PAGES to addresses 1:1, the maximum hamster db size will be slightly
less than 2^61 PAGES. Which is a size beyond this realm.

Nevertheless, the addressable RANGE is the entire 64 bit address space; it is just that the uDB digital
trie structure will take up a 2^64 bytes space by itself when nearly 2^61 rids are mapped.


The problem is aggravated by the fact that recompression of a page will very probably produce a different
pagesize then before, so the storage space either will have gaps (which need to be reused using some sort
of freelist management) or needs to be expanded/contracted with each save operation. The latter is a
VERY costly option and would only be the 'choice du jour' for STREAMING compressed storage.

An intermediate solution might be mixing both options so as to have a small freelist to manage: storage
pages are 'chunked' and compressed space allocation uses series of chunks, within which pages are contracted
and expanded, and new sections are allocated when pages would otherwise overflow the current section.


Second way
----------

A much faster alternative, which would offer slightly worse compression ratios, would be using
'targeted compression ratios', i.e. using a page chunking model where a minimum compression ratio is assumed
and when such ratio is actually achieved, the compressed page data is stored in a 'reduced pagesize' space.
If the targeted ratio could not be achieved, the page is stored 'as is' in a 'regular pagesize' space.
Multiple pagesize 'reductions' ranges can be created and managed, where a page lands in a particular range,
depending on the current compression ratio attained.

Still, the page rid must be mapped to those ranges as the destination addressing is now also dependent on
current compression ratio, which is dependent on ALL content, so a basic partitioning scheme cannot be
employed, as partitioners assume a STABLE address during the entire lifetime of the chunk / page.

Nevertheless, the 'reduced pagesize' ranges can be managed by individual device drivers when the compressor
also acts as a special purpose partitioner.

*/


#include "internal_preparation.h"



/**
* @endcond
*/

