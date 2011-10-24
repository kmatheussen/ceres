#!/bin/sh

#http://www.daa.com.au/~james/software/pygtk/

#export PYTHON="$2 $3 $4 $5 $6"

rm -fr pygtk-0.6.12
tar xvzf pygtk-0.6.12.tar.gz
cd pygtk-0.6.12
patch -p0 <../pygtk.diff
cp -f ../config.guess .
cp -f ../config.sub .

export PATH=`pwd`/../install-dir/bin:$PATH
export LD_LIBRARY_PATH=`pwd`/../install-dir/lib:$LD_LIBRARY_PATH
export CFLAGS="$CFLAGS `glib-config --cflags` -fPIC"
export LDFLAGS="$LDFLAGS `glib-config --libs` -fPIC"

if CFLAGS=-I`pwd`/../install-dir/include LDFLAGS=-L`pwd`/../install-dir/lib ./configure --prefix=$1  --with-pic --with-libglade-config=`pwd`/../install-dir/bin/libglade-config ; then
#    sed -i s:-lglib:../install-dir/lib/libglib.a: Makefile
#    sed -i s:-lgdk:../install-dir/lib/libgdk.a: Makefile
#    sed -i s:-lgtk:../install-dir/lib/libgtk.a: Makefile
#    sed -i s:-lgmodule:../install-dir/lib/libgmodule.a: Makefile
#    sed -i s:-lgthread:../install-dir/lib/libgthread.a: Makefile
    if make ; then
        sed -i s/" as"/" as2"/ gtk.py
        cd ..
        touch pygtk1_compiled
    else
        exit
    fi
else
    exit
fi
