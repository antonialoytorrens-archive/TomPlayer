include common.mk

#libraries directories 
MPLAYER-DIR=mplayer
INTERCEPT_DIR=interceptty


all : mplayer interceptty

#mplayer
$(MPLAYER-DIR)/.configured :	
	( cd $(MPLAYER-DIR);  ./configure --enable-cross-compile --cc=$(CROSS-COMPIL)gcc --as=$(CROSS-COMPIL)as --target=arm  --host-cc=gcc   --with-extraincdir=$(ROOT_DIR)/../build/usr/local/include:$(ROOT_DIR)/../build/usr/local/include/freetype2:$(ROOT_DIR)/../build/usr/local/include/ebml:$(ROOT_DIR)/../build/usr/local/include/matroska --with-extralibdir=$(ROOT_DIR)/../build/usr/local/lib --disable-profile --disable-debug --enable-gcc-check --disable-runtime-cpudetection --disable-cross-compile --enable-mencoder --enable-mplayer --disable-dynamic-plugins --disable-x11 --disable-xshape --disable-xv --disable-xvmc --disable-sdl --disable-directx --disable-win32waveout --disable-nas --disable-jpeg --disable-pnm --disable-md5sum --disable-gif --disable-gl --disable-ggi --disable-ggiwmh --disable-aa --disable-caca --disable-svga --disable-vesa --enable-fbdev --disable-dvb --disable-dvbhead --disable-dxr2 --disable-dxr3 --disable-ivtv --disable-v4l2 --enable-iconv --disable-langinfo --disable-rtc --disable-libdv --enable-ossaudio --disable-arts --disable-esd --disable-jack --disable-openal --disable-mad --disable-toolame --disable-twolame --disable-libcdio --disable-liblzo --disable-libvorbis --disable-speex --disable-tremor-internal --disable-tremor-low --disable-tremor-external --disable-theora --disable-mp3lib --disable-liba52 --disable-libdca --disable-libmpeg2 --disable-musepack --disable-faad-internal --disable-faad-external --disable-faad-fixed --disable-faac --disable-ladspa --disable-xmms --disable-dvdread --disable-dvdread-internal --disable-libdvdcss-internal --disable-dvdnav --disable-xanim --disable-real --disable-live --disable-nemesi --disable-xinerama --disable-mga --disable-xmga --disable-vm --disable-xf86keysym --disable-mlib --disable-sunaudio --disable-sgiaudio --disable-alsa --disable-tv --disable-tv-bsdbt848 --disable-tv-v4l1 --disable-tv-v4l2  --disable-tv-teletext --disable-radio-capture --disable-radio --disable-radio-v4l --disable-radio-v4l2 --disable-radio-bsdbt848 --disable-pvr --disable-fastmemcpy --disable-network --disable-winsock2 --disable-smb --disable-vidix-internal --disable-vidix-external --disable-joystick --disable-xvid --disable-x264 --disable-libnut --enable-libavutil_a --disable-libavutil_so --enable-libavcodec_a --disable-libavcodec_so --disable-libamr_nb --disable-libamr_wb --enable-libavformat_a --disable-libavformat_so --enable-libpostproc_a --disable-libpostproc_so --disable-libavcodec_mpegaudio_hp --disable-lirc --disable-lircc --disable-apple-remote --disable-gui --disable-gtk1 --disable-termcap --disable-termios --disable-3dfx --disable-s3fb --disable-tdfxfb --disable-tdfxvid --disable-xvr100 --disable-tga --disable-directfb --disable-zr --disable-bl --disable-mtrr --disable-largefiles --disable-shm --disable-select --disable-linux-devfs --disable-cdparanoia --disable-cddb --disable-big-endian --disable-bitmap-font --enable-freetype --disable-fontconfig --disable-ftp --disable-vstream --disable-w32threads --disable-ass --disable-rpath --disable-color-console --disable-fribidi --disable-enca --disable-inet6 --disable-gethostbyname2 --disable-dga1 --disable-dga2 --disable-menu --disable-qtx  --disable-macosx --disable-macosx-finder-support --disable-macosx-bundle --disable-maemo --disable-sortsub --disable-crash-debug --disable-sighandler --disable-win32dll --disable-sse --disable-sse2 --disable-ssse3 --disable-mmxext --disable-3dnow --disable-3dnowext --disable-cmov --disable-fast-cmov --disable-altivec --disable-armv5te --disable-armv6 --disable-iwmmxt --disable-mmx --enable-png  && touch  .configured )
	
#--enable-static
mplayer : $(MPLAYER-DIR)/.configured
	( cd $(MPLAYER-DIR) && make )


mplayer-distclean : 	
	( cd $(MPLAYER-DIR) && make distclean &&  rm -f .configured)


$(INTERCEPT_DIR).configured :
	( cd $(INTERCEPT_DIR); ac_cv_file___dev_ptmx_=yes ./configure --host=arm-linux --disable-dev-ptc --enable-dev-ptmx=yes)

interceptty: $(INTERCEPT_DIR).configured
	(cd $(INTERCEPT_DIR) && make )

