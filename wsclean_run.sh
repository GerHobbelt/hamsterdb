#! /bin/bash

# brute force everything to UNIX with spaces:
wsclean -i -e -x -U -v  $( find . -type f -iname '*' -a ! -ipath '*/boost/*' -a ! -iname 'makefile*' )

# make sure Makefiles are tabbed:
wsclean -i -r -x -U -T 8 -v  $( find . -type f -iname 'Makefile*' -a ! -ipath '*/boost/*' )

# and MSVC files are MSDOS:
wsclean -i -r -x -W -v  $( find . -type f -iname '*.sln' -a ! -ipath '*/boost/*' )
wsclean -i -r -x -W -v  $( find . -type f -iname '*.vc*proj' -a ! -ipath '*/boost/*' )
wsclean -i -r -x -W -v  $( find . -type f -iname '*.bat' -a ! -ipath '*/boost/*' )

