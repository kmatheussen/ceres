#!/bin/sh


#http://www.daa.com.au/~james/software/libglade/
rm -fr libglade-0.17
tar xvzf libglade-0.17.tar.gz
cd libglade-0.17
export PATH=`pwd`/../install-dir/bin:$PATH
export LD_LIBRARY_PATH=`pwd`/../install-dir/lib:$LD_LIBRARY_PATH
export CFLAGS="$CFLAGS `glib-config --cflags` -fPIC"
export LDFLAGS="$LDFLAGS `glib-config --libs` -fPIC"

if ./configure --prefix=`pwd`/../install-dir --enable-shared=false --with-pic && make && make install ; then
    chmod a+rx libglade-config
    cd ..
    touch libglade_compiled
else
    exit
fi


