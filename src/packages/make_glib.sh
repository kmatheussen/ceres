#!/bin/sh


rm -fr glib-1.2.10
tar xvzf glib-1.2.10.tar.gz
cd glib-1.2.10
patch -p0 <../glib.diff
#patch -p0 <../glib2.diff
cp -f ../config.guess .
cp -f ../config.sub .
export PATH=`pwd`/../install-dir/bin:$PATH
export LD_LIBRARY_PATH=`pwd`/../install-dir/lib:$LD_LIBRARY_PATH

if ./configure --prefix=`pwd`/../install-dir && make && make install ; then
    cd ..
    touch glib_compiled
else
    exit
fi


