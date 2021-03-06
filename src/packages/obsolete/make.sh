#!/bin/sh


if [ ! -f libglade_installed ] ; then
    sh make_libxml.sh
fi

#http://www.daa.com.au/~james/software/libglade/
if [ ! -f libglade_installed ] ; then
    tar xvzf libglade-0.17.tar.gz
    cd libglade-0.17
    if ./configure --prefix=$1 && make && make install  ; then
	cd ..
	touch libglade_installed

#http://www.daa.com.au/~james/software/pygtk/
	if [ ! -f pygtk_installed ] ; then
	    tar xvzf pygtk-0.6.12.tar.gz
	    cd pygtk-0.6.12
	    export CFLAGS="$CFLAGS `glib-config --cflags`"
	    export LDFLAGS="$LDFLAGS `glib-config --libs`"
	    if ./configure --prefix=$1 --with-libglade-config=$1/bin/libglade-config && make && make install ; then
		cd ..
		touch pygtk_installed
	    else
		exit
	    fi
	fi


    else
	exit
    fi
fi






