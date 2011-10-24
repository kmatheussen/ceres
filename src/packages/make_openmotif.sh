#!/bin/sh

rm -fr openmotif-2.3.2
tar xvzf openmotif-2.3.2.tar.gz
cd openmotif-2.3.2

export CFLAGS="$CFLAGS -fPIC"
export LDFLAGS="$LDFLAGS -fPIC"

unset LANG
if ./configure --prefix=$1 --with-pic && make ; then
    cd ..
    touch openmotif_compiled
else
    exit
fi
