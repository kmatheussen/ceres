

all_compiled: libxml_compiled libglade_compiled pygtk1_compiled openmotif_compiled sndlib_compiled
	touch all_compiled


clean:
	rm -fr libxml-1.8.17 libglade-0.17 pygtk-0.6.12 sndlib libxml_compiled libglade_compiled pygtk1_compiled openmotif_compiled glib-1.2.10 glib_compiled gtk+-1.2.10 gtk+_compiled openmotif-2.3.2 sndlib_compiled all_compiled install-dir/*


install:
	cd pygtk-0.6.12 && make install
	cp install-dir/lib/libgtk-1.2.so.0 install-dir/lib/libgdk-1.2.so.0 install-dir/lib/libgmodule-1.2.so.0 install-dir/lib/libglib-1.2.so.0 install-dir/lib/libgthread.so $(PYGTK1PATH)/lib/


openmotif_compiled:
	bash make_openmotif.sh


pygtk1_compiled: libglade_compiled gtk+_compiled glib_compiled ../Makefile
	bash make_pygtk1.sh $(PYGTK1PATH)


libglade_compiled: libxml_compiled gtk+_compiled glib_compiled
	bash make_libglade.sh $(PYGTK1PATH)


libxml_compiled: glib_compiled
	bash make_libxml.sh


gtk+_compiled: glib_compiled
	bash make_gtk+.sh $(PYGTK1PATH)


glib_compiled:
	bash make_glib.sh $(PYGTK1PATH)


#http://ccrma.stanford.edu/software/snd/sndlib/
sndlib_compiled:
	echo "\
	rm -fr sndlib;\
	tar xvzf sndlib.tar.gz;\
	cd sndlib;\
	./configure $(SNDLIBWITHJACK) --with-pic --without-guile;\
	make;\
	touch sndlib_compiled\
	" >makesource.sh
	sh makesource.sh
	rm makesource.sh
