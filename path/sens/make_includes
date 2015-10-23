#!/bin/sh
# Creates a link to all header files (i.e. files 
# named *.h) under the current directory. Duplicated
# header files will appear in include/make_includes.err

mkdir -p include
cd include
rm -f *.h make_includes.err
ln -f -s `find .. -name *.h` . 2>make_includes.err
