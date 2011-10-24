#!/bin/sh

#ftp://xmlsoft.org/libxml2/old/

rm -fr libxml-1.8.17
tar xvzf libxml-1.8.17.tar.gz
cd libxml-1.8.17
patch -p0 <../libxml.diff

export PATH=`pwd`/../install-dir/bin:$PATH
export LD_LIBRARY_PATH=`pwd`/../install-dir/lib:$LD_LIBRARY_PATH
export CFLAGS="$CFLAGS `glib-config --cflags` -fPIC"
export LDFLAGS="$LDFLAGS `glib-config --libs` -fPIC"

unset LANG
if ./configure --prefix=`pwd`/../install-dir --enable-shared=false --with-pic && make && make install; then
    cd ..
    touch libxml_compiled
else
    exit
fi
