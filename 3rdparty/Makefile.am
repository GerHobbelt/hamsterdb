
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


#
# mongoose lib, done the autoconf/automake way
#

# INCLUDES = 
AM_CPPFLAGS = -I$(top_srcdir)/include

noinst_LTLIBRARIES = libmongoose.la

libmongoose_la_SOURCES = mongoose/mongoose.c


