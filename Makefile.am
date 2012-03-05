ACLOCAL_AMFLAGS	= -I m4 

SUBDIRS = 3rdparty src include samples tools unittests java
if ENABLE_SERVER
if ENABLE_REMOTE
#SUBDIRS += server
endif
endif # ENABLE_REMOTE && ENABLE_SERVER

DOCUMENTATION_FILES = documentation/Doxyfile            \
             documentation/hamsterdb-main-low.jpg       \
             documentation/hamsterdb-main.jpg           \
		documentation/architecture.doc \
		documentation/appendix_gpl2.doc \
		documentation/appendix_gpl3.doc \
		documentation/appendix_gfdl.doc \
             documentation/faq.doc 		\
             documentation/got_geek.doc 		\
             documentation/history.doc 		\
             documentation/index.doc 		\
             documentation/samples.doc 		\
             documentation/starting.doc 		\
		documentation/tuning.doc \
		documentation/trouble.doc \
             documentation/hamsterdb.css                \
             documentation/header.html                  \
             documentation/footer.html                  \
             documentation/Doxyfile                     \
             documentation/Doxyfile.BFC                     \
             documentation/Doxyfile.int                     \
             documentation/page-filters.ppt                     \
             documentation/href.pl                      \
             documentation/Doxyfile.www


EXTRA_DIST = $(DOCUMENTATION_FILES)                     \
             ChangeLog                                  \
             hamsterdb.spec                             \
             AUTHORS                                    \
             bootstrap                             \
             uncrustify.cfg                             \
             uncrustify.rupp.cfg                        \
             README                                     \
             CREDITS                                    \
             COPYING.GPL2                               \
             COPYING.GPL3                               \
             3rdparty                                   \
<<<<<<< HEAD
             src/*.cc src/*.h src/Makefile.am   		\
             src/Makefile.in                            \
=======
             m4/ax_*.m4                            \
             build/scripts/*                            \
>>>>>>> flash-bang-grenade
             src/protocol/messages.proto                \
             src/protocol                               \
             src/server                                 \
             java/*                                     \
             tools/*.h                                  \
             tools/hamzilla.config                      \
<<<<<<< HEAD
             unittests/*.h                              \
             unittests/*.hpp                            \
             unittests/*.cpp                            \
             unittests/Makefile.in                      \
             unittests/Makefile.am                      \
             unittests/recovery.pl                      \
=======
>>>>>>> flash-bang-grenade
             unittests/data/*                           \
             unittests/bfc.doc                          \
             win32/README.TXT                           \
<<<<<<< HEAD
             win32/hamsterdb.sln                        \
             win32/dll.vcproj                           \
             win32/lib.vcproj                           \
             win32/sample_db1.vcproj                    \
             win32/sample_db2.vcproj                    \
             win32/sample_db3.vcproj                    \
             win32/sample_db4.vcproj                    \
             win32/sample_env1.vcproj                   \
             win32/sample_env2.vcproj                   \
             win32/unittests.vcproj                     \
             java                                       \
             dotnet                                     \
             contrib                                    \
             contrib/wince/*
=======
             win32/msvc2008/hamsterdb.sln                        \
             win32/msvc2008/ham_dump.vcproj                 \
             win32/msvc2008/ham_info.vcproj                 \
             win32/msvc2008/3rdparty_aes.vcproj                 \
             win32/msvc2008/3rdparty_zlib.vcproj                 \
             win32/msvc2008/hamsterDB_dll.vcproj                 \
             win32/msvc2008/hamsterDB_lib.vcproj                 \
             win32/msvc2008/sample_db1.vcproj                    \
             win32/msvc2008/sample_db2.vcproj                    \
             win32/msvc2008/sample_db3.vcproj                    \
             win32/msvc2008/sample_db4.vcproj                    \
             win32/msvc2008/sample_db5.vcproj                    \
             win32/msvc2008/sample_db6.vcproj                    \
             win32/msvc2008/sample_db7.vcproj                    \
             win32/msvc2008/sample_env1.vcproj                   \
             win32/msvc2008/sample_env2.vcproj                   \
             win32/msvc2008/sample_env3.vcproj                   \
             win32/msvc2008/sample_env4.vcproj                   \
             win32/msvc2008/unittests.vcproj                     \
             win32/msvc2008/bfc-sample.vcproj                    \
             win32/msvc2005/hamsterdb.sln                        \
             win32/msvc2005/ham_dump.vcproj                 \
             win32/msvc2005/ham_info.vcproj                 \
             win32/msvc2005/3rdparty_aes.vcproj                 \
             win32/msvc2005/3rdparty_zlib.vcproj                 \
             win32/msvc2005/hamsterDB_dll.vcproj                 \
             win32/msvc2005/hamsterDB_lib.vcproj                 \
             win32/msvc2005/sample_db1.vcproj                    \
             win32/msvc2005/sample_db2.vcproj                    \
             win32/msvc2005/sample_db3.vcproj                    \
             win32/msvc2005/sample_db4.vcproj                    \
             win32/msvc2005/sample_db5.vcproj                    \
             win32/msvc2005/sample_db6.vcproj                    \
             win32/msvc2005/sample_db7.vcproj                    \
             win32/msvc2005/sample_env1.vcproj                   \
             win32/msvc2005/sample_env2.vcproj                   \
             win32/msvc2005/sample_env3.vcproj                   \
             win32/msvc2005/sample_env4.vcproj                   \
             win32/msvc2005/unittests.vcproj                     \
             wince/README.TXT                           \
             wince/hamsterdb.sln                        \
             wince/lib.vcproj                           \
             wince/dll.vcproj                           \
             wince/3rdparty_aes.vcproj                 \
             wince/3rdparty_zlib.vcproj                 \
             wince/sample_db1.vcproj                    \
             wince/sample_env1.vcproj                   \
             wince/sample_env2.vcproj
>>>>>>> flash-bang-grenade

doc: documentation/Doxyfile
	VERSION=$(VERSION) ;            \
	export VERSION;                 \
	doxygen documentation/Doxyfile
	@cp documentation/*.jpg documentation/html/
#	@sed -e "s/\$$VERSION/$(VERSION)/g" doxygen_manual.tex >../latex/doxygen_manual.tex
#	@sed -e "s/\$$VERSION/$(VERSION)/g" doxygen.sty >../latex/doxygen.sty
#	@epstopdf doxygen_logo.eps --outfile=../latex/doxygen_logo.pdf


internal_doc: documentation/Doxyfile.int
	VERSION=$(VERSION) ;            \
	export VERSION;                 \
	doxygen documentation/Doxyfile.int
#	@cp documentation/*.jpg documentation/internal/html/

bfc_doc: documentation/Doxyfile.bfc
	VERSION=$(VERSION) ;            \
	export VERSION;                 \
	doxygen documentation/Doxyfile.bfc
#	@cp documentation/hamsterdb-main*.* documentation/html/


all_doc: doc bfc_doc internal_doc


html-dist: doc
	tar cvzf hamsterdb-html-documentation.$(VERSION).tar.gz documentation/html

update_version:
	VERSION=$(VERSION) ;            \
	export VERSION;                 \
	sed 's/hamsterdb-[0-9]*.[0-9]*.[0-9]*.dll/hamsterdb-$(VERSION).dll/' win32/dll.vcproj > win32/_dll.vcproj
	mv win32/_dll.vcproj win32/dll.vcproj
	sed 's/libhamsterdb-[0-9]*.[0-9]*.[0-9]*.lib/libhamsterdb-$(VERSION).lib/' win32/lib.vcproj > win32/_lib.vcproj
	mv win32/_lib.vcproj win32/lib.vcproj
	sed 's/hamserver-[0-9]*.[0-9]*.[0-9]*.dll/hamserver-$(VERSION).dll/' win32/server_dll.vcproj > win32/_server_dll.vcproj
	mv win32/_server_dll.vcproj win32/server_dll.vcproj
	sed 's/libhamserver-[0-9]*.[0-9]*.[0-9]*.lib/libhamserver-$(VERSION).lib/' win32/server_lib.vcproj > win32/_server_lib.vcproj
	mv win32/_server_lib.vcproj win32/server_lib.vcproj
	sed 's/Version: [0-9]*.[0-9]*.[0-9]*/Version: $(VERSION)/' hamsterdb.spec > _hamsterdb.spec
	mv _hamsterdb.spec hamsterdb.spec

test:
	cd unittests && make && ./test
if ENABLE_JAVA
	cd java && make test
endif

.PHONY: doc all_doc internal_doc bfc_doc html-dist test


