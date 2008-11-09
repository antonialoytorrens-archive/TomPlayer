include common.mk

#libraries directories 
LIBPNG-DIR=libpng-1.2.29
ZLIB-DIR=zlib-1.2.3
LIBJPEG-DIR=libjpeg-6b
DEVIL-DIR=DevIL-1.6.8
FREETYPE-DIR=freetype-2.3.5
TSLIB-DIR=tslib-1.0
LIBZIP-DIR=libzip-0.8
INIPARSER-DIR=iniparser3.0b
#DIRECTFB-DIR=DirectFB-1.2.0
DIRECTFB-DIR=DirectFB-1.3.0

GLIB-DIR=glib-2.13.7
ATK-DIR=atk-1.9.1
FONTCONF-DIR=fontconfig-2.5.91
CAIRO-DIR=cairo-1.2.6
EXPAT-DIR=expat-2.0.0
PANGO-DIR=pango-1.16.4
GTK-DIR=gtk+-2.10.14

LIB-DISTCLEAN-ADD= glib-distclean atk-distclean  fontconfig-distclean  cairo-distclean expat-distclean  pango-distclean  gtk-distclean 

LIB-DISTCLEAN= libpng-distclean  zlib-distclean libjpeg-distclean devil-distclean freetype-distclean tslib-distclean libzip-distclean iniparser-distclean directfb-distclean $(LIB-DISTCLEAN-ADD)



#directfb : libjpeg libpng  zlib 
#devil : libjpeg libpng  zlib
 
all : zlib libpng libjpeg devil freetype tslib libzip  iniparser directfb

gtk : all glib expat atk fontconfig cairo pango gtk 


distclean : $(LIB-DISTCLEAN)
	rm -rf $(INSTALL_DIR)/$(PREFIX)

#Libpng

libpng : $(LIBDIR)/libpng12.so


$(LIBPNG-DIR)/Makefile : 
	( cd $(LIBPNG-DIR) ;  $(CONFIGURE) $(CONFIG-OPTS) --disable-static )        
	
$(LIBDIR)/libpng12.so : $(LIBPNG-DIR)/Makefile 
	( cd $(LIBPNG-DIR) && make && make install DESTDIR=$(INSTALL_DIR))
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 
	
libpng-clean : $(LIBPNG-DIR)/Makefile 	
	( cd $(LIBPNG-DIR) && make clean)

libpng-distclean :	
	( cd $(LIBPNG-DIR) ; make distclean )
	
#Zlib 
$(ZLIB-DIR)/.configured :	
	( cd $(ZLIB-DIR) ;  CC=$(CROSS-COMPIL)gcc ./configure --shared )        
	

zlib : $(LIBDIR)/libz.so

$(LIBDIR)/libz.so : $(ZLIB-DIR)/.configured
	( cd $(ZLIB-DIR) && make && make install prefix=$(INSTALL_DIR)/usr/local/ )
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 


zlib-distclean : 	
	( cd $(ZLIB-DIR) && make distclean &&  rm -f .configured)
	

#libjpeg
$(LIBJPEG-DIR)/Makefile :
	( cd $(LIBJPEG-DIR) ; $(CONFIGURE) $(CONFIG-OPTS) --enable-shared --disable-static )        
	

libjpeg : $(LIBDIR)/libjpeg.so

$(LIBDIR)/libjpeg.so : $(LIBJPEG-DIR)/Makefile 
	( cd $(LIBJPEG-DIR) && make all install DESTDIR=$(INSTALL_DIR))
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 
	

libjpeg-distclean :	
	( cd $(LIBJPEG-DIR) ; make distclean )

#Devil
$(DEVIL-DIR)/Makefile : 	
	(cd $(DEVIL-DIR); $(CONFIGURE) $(CONFIG-OPTS) --disable-altivec --disable-sse --disable-sse2 --disable-sse3 --disable-dcx --disable-doom --disable-hdr --disable-lif --disable-lif --disable-mdl --disable-mng --disable-pcx --disable-pcd --disable-pic --disable-pix --disable-pnm --disable-psd --disable-psp --disable-pxr --disable-raw --disable-sgi --disable-tga --disable-xpm --disable-wal --disable-allegro --disable-directx8 --disable-directx9 --disable-opengl --disable-sdl --disable-win32 --disable-mng --disable-tiff --disable-lcms --with-x=no )
	

devil :  $(DEVIL-DIR)/Makefile
	make -C $(DEVIL-DIR) all install  DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 
	
devil-distclean :
	make -C $(DEVIL-DIR) distclean
	rm -f $(DEVIL-DIR)/Makefile
	

#Freetype

$(FREETYPE-DIR)/builds/unix/unix-cc.mk :
	(cd $(FREETYPE-DIR); $(CONFIGURE) $(CONFIG-OPTS) --disable-static )
	

freetype : $(LIBDIR)/libfreetype.so

$(LIBDIR)/libfreetype.so : $(FREETYPE-DIR)/builds/unix/unix-cc.mk 
	make -C $(FREETYPE-DIR) 
	make -C $(FREETYPE-DIR)  install DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 

freetype-distclean :
	make -C $(FREETYPE-DIR)  distclean
	

# tslib

tslib : $(LIBDIR)/libts.so

$(TSLIB-DIR)/configure :
	(cd $(TSLIB-DIR) && ./autogen.sh )
	

#$(TSLIB-DIR)/Makefile : $(TSLIB-DIR)/configure
$(TSLIB-DIR)/Makefile : 
	(cd $(TSLIB-DIR); $(CONFIGURE) $(CONFIG-OPTS) --enable-linear-h2200=no --enable-ucb1x00=no  --enable-corgi=no --enable-collie=no --enable-mk712=no --enable-arctic2=no)
	(cd $(TSLIB-DIR) ;cat config.h | grep -v rpl_malloc > tmp.h && cp tmp.h config.h)
	

$(LIBDIR)/libts.so : $(TSLIB-DIR)/Makefile
	make -C  $(TSLIB-DIR) all install  DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 

tslib-distclean :
	make -C $(TSLIB-DIR) distclean	


#libzip

$(LIBZIP-DIR)/Makefile :
	(cd $(LIBZIP-DIR); $(CONFIGURE) $(CONFIG-OPTS) -disable-static)

$(LIBDIR)/libzip.so : $(LIBZIP-DIR)/Makefile 
	make -C  $(LIBZIP-DIR) all install  DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 

libzip : $(LIBDIR)/libzip.so 

libzip-distclean:
	make -C  $(LIBZIP-DIR) distclean


#iniparser

iniparser : $(LIBDIR)/libiniparser.so

$(LIBDIR)/libiniparser.so :
	make -C $(INIPARSER-DIR) CC=$(CROSS-COMPIL)gcc  
	cp $(INIPARSER-DIR)/src/*.h $(INCDIR)
	cp $(INIPARSER-DIR)/libiniparser.so.0 $(LIBDIR)/
	(cd  $(LIBDIR)/ && ln -s libiniparser.so.0 libiniparser.so)	

iniparser-distclean :
	make -C $(INIPARSER-DIR)  veryclean
	rm -f $(INCDIR)/iniparser.h
	rm -f $(INCDIR)/dictionary.h
	rm -f $(LIBDIR)/libiniparser.so.0
	rm -f $(LIBDIR)/libiniparser.so


# DirectFB

directfb :  $(LIBDIR)/libdirectfb.so

$(DIRECTFB-DIR)/Makefile :
	(cd $(DIRECTFB-DIR); FREETYPE_CFLAGS="-I$(INCDIR)" FREETYPE_LIBS="-lfreetype" LIBS=-lz TSLIB_CFLAGS="-I$(INCDIR)" TSLIB_LIBS="-lts" $(CONFIGURE) $(CONFIG-OPTS) -with-inputdrivers=tslib --with-gfxdrivers=none  --disable-x11 --disable-video4linux --disable-video4linux2  --enable-zlib --with-inputdrivers=tslib,linuxinput )

$(LIBDIR)/libdirectfb.so : $(DIRECTFB-DIR)/Makefile
	make -C  $(DIRECTFB-DIR) all install  DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 
	

directfb-distclean :	
	make -C $(DIRECTFB-DIR)  distclean


# Glib

glib :  $(LIBDIR)/libglib-2.0.so

$(GLIB-DIR)/Makefile :
	(cd $(GLIB-DIR); $(CONFIGURE) $(CONFIG-OPTS) --cache-file=arm_cache.conf)

$(LIBDIR)/libglib-2.0.so : $(GLIB-DIR)/Makefile
	make -C  $(GLIB-DIR) all install  DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 
	

# Warning to dep libs in la files from glib...

glib-distclean :	
	make -C $(GLIB-DIR)  distclean


# Atk

atk :  $(LIBDIR)/libatk-1.0.so 

$(ATK-DIR)/Makefile :
	(cd $(ATK-DIR); $(CONFIGURE) $(CONFIG-OPTS))

$(LIBDIR)/libatk-1.0.so : $(ATK-DIR)/Makefile
	make -C  $(ATK-DIR) all install  DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 


atk-distclean :	
	make -C $(ATK-DIR)  distclean

# expat

expat :  $(LIBDIR)/libexpat.so 

$(EXPAT-DIR)/Makefile :
	(cd $(EXPAT-DIR); $(CONFIGURE) $(CONFIG-OPTS) )

$(LIBDIR)/libexpat.so : $(EXPAT-DIR)/Makefile
	make -C  $(EXPAT-DIR) all install  DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 
	

expat-distclean :	
	make -C $(EXPAT-DIR)  distclean	

# Fontconfig

fontconfig :  $(LIBDIR)/libfontconfig.so 

$(FONTCONF-DIR)/Makefile :
	(cd $(FONTCONF-DIR); $(CONFIGURE) $(CONFIG-OPTS) LDFLAGS="$(LDFLAGS) -lz -lexpat" --enable-libxml2=no --with-arch=arm --with-freetype-config=$(INSTALL_DIR)/$(PREFIX)/bin/freetype-config )

$(LIBDIR)/libfontconfig.so : $(FONTCONF-DIR)/Makefile
	make -C  $(FONTCONF-DIR) all install  DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 

fontconfig-distclean :	
	make -C $(FONTCONF-DIR)  distclean	
	
# Cairo
cairo :  $(LIBDIR)/libcairo.so 

$(CAIRO-DIR)/Makefile :
	(cd $(CAIRO-DIR); $(CONFIGURE) directfb_CFLAGS="-I$(INCDIR)/directfb" directfb_LIBS="-ldirectfb" FONTCONFIG_CFLAGS="-I$(INCDIR)" FONTCONFIG_LIBS="-lfontconfig"  FREETYPE_CFLAGS="-I$(INCDIR)" FREETYPE_LIBS="-lfreetype" $(CONFIG-OPTS) --enable-xlib=no --enable-xlib-xrender=no --enable-ps=no --enable-pdf=no --enable-svg=no --enable-atsui=no --enable-gcov=no --enable-test-surfaces=no --enable-directfb=yes)

$(LIBDIR)/libcairo.so : $(CAIRO-DIR)/Makefile
	make -C  $(CAIRO-DIR) all install  DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 
	


cairo-distclean :	
	make -C $(CAIRO-DIR)  distclean

# Pango
pango :  $(LIBDIR)/libpango-1.0.so 

$(PANGO-DIR)/Makefile :
	(cd $(PANGO-DIR); $(CONFIGURE) LDFLAGS="$(LDFLAGS) -lz -lfontconfig -lfusion -ldirect -ldirectfb -lexpat -lpng12 " FONTCONFIG_CFLAGS="-I$(INCDIR)" FONTCONFIG_LIBS="-lfontconfig" GLIB_CFLAGS="-I$(INCDIR)" GLIB_CFLAGS="-lglib-2.0" CAIRO_CFLAGS="-I$(INCDIR)/cairo" CAIRO_LIBS="-lcairo" $(CONFIG-OPTS) --with-x=no )

$(LIBDIR)/libpango-1.0.so : $(PANGO-DIR)/Makefile
	make -C  $(PANGO-DIR) all install  DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 
	

pango-distclean :	
	make -C $(PANGO-DIR)  distclean	


# GTK
gtk :  $(LIBDIR)/libgtk-directfb-2.0.so

$(GTK-DIR)/Makefile :
	(cd $(GTK-DIR); $(CONFIGURE) PKG_CONFIG_PATH=$(LIBDIR)/pkgconfig CFLAGS="$(CFLAGS)  -I$(INCDIR)/directfb -I$(INCDIR)/cairo  -I$(INCDIR)/pango-1.0 -I$(INCDIR)/glib-2.0 -I$(LIBDIR)/glib-2.0/include  -I$(INCDIR)/atk-1.0" LDFLAGS="$(LDFLAGS)  -lgmodule-2.0 -lglib-2.0 -lgobject-2.0 -lz -lfontconfig -lfusion -ldirect -ldirectfb -lexpat -lpng12 -lpango-1.0 -lpangoft2-1.0" CPPFLAGS="-I$(INCDIR)" $(CONFIG-OPTS) --without-libtiff -with-gdktarget=directfb --without-x --enable-fbmanager)

$(LIBDIR)/libgtk-directfb-2.0.so : $(GTK-DIR)/Makefile
	make -C  $(GTK-DIR) all install  DESTDIR=$(INSTALL_DIR)
	./patch_la.sh  $(PREFIX)/lib $(INSTALL_DIR) 
	

gtk-distclean :	
	make -C $(GTK-DIR)  distclean		


examples :
	(cd DirectFB-examples-1.0.1;  DIRECTFB_CFLAGS="-I$(INCDIR) -I$(INCDIR)/directfb" DIRECTFB_LIBS="-L$(LIBDIR)/directfb-1.2-0 -ldirectfb  -ldirect -lfusion -lz" $(CONFIGURE) $(CONFIG-OPTS) )
	make -C  DirectFB-examples-1.0.1 all

