#!/bin/sh


rm -fr gtk+-1.2.10
tar xvzf gtk+-1.2.10.tar.gz
cd gtk+-1.2.10
cp -f ../config.guess .
cp -f ../config.sub .

export PATH=`pwd`/../install-dir/bin:$PATH
export LD_LIBRARY_PATH=`pwd`/../install-dir/lib:$LD_LIBRARY_PATH

if ./configure --prefix=`pwd`/../install-dir --with-glib-prefix=`pwd`/../install-dir/ --with-glib-exec-prefix=`pwd`/../install-dir/ && make && make install ; then
    cd ..
    touch gtk+_compiled
else
    exit
fi


