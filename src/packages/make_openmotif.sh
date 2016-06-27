#!/bin/sh

rm -fr openmotif-2.3.2
tar xvzf openmotif-2.3.2.tar.gz
cd openmotif-2.3.2

echo "$(echo '%option main' | cat - tools/wml/wmluiltok.l)" > tools/wml/wmluiltok.l

export CFLAGS="$CFLAGS -fPIC"
export LDFLAGS="$LDFLAGS -fPIC"

unset LANG
if ./configure --prefix=$1 --with-pic && make -j8 ; then
    cd ..
    touch openmotif_compiled
else
    exit
fi
