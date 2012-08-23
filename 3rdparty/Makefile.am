
if ENABLE_ENCRYPTION
SUBDIRS = aes
else
SUBDIRS = 
endif

if ENABLE_COMPRESSION
if SYSTEM_ZLIB
# nop
else
SUBDIRS += zlib
endif
endif

if ENABLE_SERVER
SUBDIRS += json
endif


# INCLUDES = 
AM_CPPFLAGS = -DNO_DUMMY_DECL 
              -I$(top_srcdir)/include

#
# mongoose lib, done the autoconf/automake way
#

noinst_LTLIBRARIES = libmongoose.la

libmongoose_la_SOURCES = mongoose/mongoose_ex.c



#
# zlib lib, done the autoconf/automake way
#

noinst_LTLIBRARIES += libzlib.la

libzlib_la_SOURCES = \
					inffast.c \
					adler32.c \
					compress.c \
					crc32.c \
					deflate.c \
					gzclose.c \
					gzlib.c \
					gzread.c \
					gzwrite.c \
					infback.c \
					inffast.c \
					inflate.c \
					inftrees.c \
					trees.c \
					uncompr.c \
					zutil.c
