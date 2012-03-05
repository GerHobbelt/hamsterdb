
if ENABLE_REMOTE
SUBDIRS = protocol
if ENABLE_SERVER 
SUBDIRS += server
endif # ENABLE_SERVER
endif # ENABLE_REMOTE

<<<<<<< HEAD
AM_CPPFLAGS = -I../include -I$(top_builddir)/3rdparty/aes $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_THREAD_LDFLAGS)

lib_LTLIBRARIES = libhamsterdb.la

EXTRA_DIST = os_win32.cc

libhamsterdb_la_SOURCES = log.cc \
			cursor.cc \
			blob.cc \
			btree.cc \
			btree_check.cc \
			btree_enum.cc \
			btree_erase.cc \
			btree_find.cc \
			btree_insert.cc \
			btree_key.cc \
			cache.cc \
			db.cc \
			env.cc \
			error.cc \
			extkeys.cc \
			freelist.cc \
			freelist_v2.cc \
			freelist_statistics.cc \
			hamsterdb.cc \
			remote.cc \
			mem.cc \
			os_posix.cc \
			page.cc \
			btree_stats.cc \
			txn.cc \
			txn_cursor.cc \
			util.cc \
			btree_cursor.cc \
			journal.cc \
			changeset.cc \
			device.cc

libhamsterdb_la_LDFLAGS = -version-info 3:0:0 -lboost_thread-mt -lpthread 
=======
AM_CFLAGS = -I../include -I$(top_builddir)/3rdparty/aes

lib_LTLIBRARIES = libhamsterdb.la

EXTRA_DIST = os_win32.c


libhamsterdb_la_SOURCES = \
                backend.h \
                blob.c \
                blob.h \
                btree.c \
                btree.h \
                btree_check.c \
                btree_cursor.c \
                btree_cursor.h \
                btree_enum.c \
                btree_erase.c \
                btree_find.c \
                btree_insert.c \
                cache.c \
                cache.h \
                config.h \
                cuckoo_hash.c \
                cuckoo_hash.h \
                cuckoo_hash_cursor.h \
                cursor.h \
                db.c \
                db.h \
                device.c \
                device.h \
                devices/cachedev.c \
                devices/compression.c \
                devices/stream_compression.c \
                devices/crypter.c \
                devices/db_partitioner.c \
                devices/event_sender.c \
                devices/field_partitioner.c \
                devices/file_per_rid.c \
                devices/flashmem.c \
                devices/flatfile.c \
                devices/graph_mgr.c \
                devices/https.c \
                devices/key_partitioner.c \
                devices/logger_swapper.c \
                devices/mallocmem.c \
                devices/rid_partitioner.c \
                ham_endian.h \
                env.c \
                env.h \
                error.c \
                error.h \
                extkeys.c \
                extkeys.h \
                freelist.c \
                freelist.h \
                freelist_v2.c \
                freelist_v2.h \
                freelist_statistics.c \
                freelist_statistics.h \
                hamsterdb.c \
                internal_fwd_decl.h \
                keys.c \
                keys.h \
                log.c \
                log.h \
                mem.c \
                mem.h \
                os.h \
                os_posix.c \
                page.c \
                page.h \
		remote.c \
                serial.h \
                statistics.c \
                statistics.h \
                txn.c \
                txn.h \
                udb_digital_trie.c \
                util.c \
                util.h \
                version.h

libhamsterdb_la_LDFLAGS = -version-info 2:0:0
>>>>>>> flash-bang-grenade

if ENABLE_ENCRYPTION
libhamsterdb_la_LDFLAGS += $(top_builddir)/3rdparty/aes/libaes.la
endif

if ENABLE_COMPRESSION
if SYSTEM_ZLIB
libhamsterdb_la_LDFLAGS += -lz
else
libhamsterdb_la_LDFLAGS += $(top_builddir)/3rdparty/zlib/libzlib.la
endif
endif

if ENABLE_REMOTE
libhamsterdb_la_LDFLAGS += -lcurl
libhamsterdb_la_LDFLAGS += $(top_builddir)/src/protocol/libprotocol.la
<<<<<<< HEAD
CFLAGS += -DHAM_ENABLE_REMOTE
CXXFLAGS += -DHAM_ENABLE_REMOTE
=======
AM_CFLAGS += -DHAM_ENABLE_REMOTE
>>>>>>> flash-bang-grenade
endif
LIBS = $(BOOST_THREAD_LIBS)

