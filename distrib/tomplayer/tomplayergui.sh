#!/bin/sh

#Entering tomplayer directory installation
TOMPLAYER_DIR=`echo $0 | sed 's/tomplayergui.sh//'` 
cd $TOMPLAYER_DIR

#To be sure to get absolute path
TOMPLAYER_DIR=`pwd`


echo "Killing ttn..."

killall -9 mplayer
killall -9 ttn
killall -9 mp3d 
killall -9 a2dpd

rm /mnt/sdcard/dirty_fs 

/etc/rc.usbkill 

killall -9 clmapp
killall -9 ttsserver 
killall -9 suntime 


killall -9 refresh_wdg 
./refresh_wdg &

#convert in UNIX text format the used configuration file
cp -f conf/tomplayer.ini /tmp/tomplayer.ini
dos2unix /tmp/tomplayer.ini

#!/bin/sh
export FRAMEBUFFER=/dev/fb
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb
export TSLIB_TSDEVICE=/dev/ts
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_PLUGINDIR=/usr/lib/ts


export LIB_TOMPLAYER=${TOMPLAYER_DIR}/lib
export CONF_TOMPLAYER=${TOMPLAYER_DIR}/conf
cd /etc
ln -sf ${CONF_TOMPLAYER}/ts.conf ts.conf

mkdir -p /usr/lib
cd /usr/lib
ln -sf ${LIB_TOMPLAYER}/pkgconfig pkgconfig
ln -sf ${LIB_TOMPLAYER}/directfb-1.2-0 directfb-1.2-0
ln -sf ${LIB_TOMPLAYER}/ts ts
ln -sf ${LIB_TOMPLAYER}/libfusion-1.2.so.0.0.0 libfusion-1.2.so.0.0.0
ln -sf ${LIB_TOMPLAYER}/libILUT.la libILUT.la
ln -sf ${LIB_TOMPLAYER}/libts-0.0.so.0.1.1 libts-0.0.so.0.1.1
ln -sf ${LIB_TOMPLAYER}/libfreetype.so.6.3.16 libfreetype.so.6.3.16
ln -sf ${LIB_TOMPLAYER}/libzip.la libzip.la
ln -sf ${LIB_TOMPLAYER}/libIL.la libIL.la
ln -sf ${LIB_TOMPLAYER}/libILU.la libILU.la
ln -sf ${LIB_TOMPLAYER}/libdirect.la libdirect.la
ln -sf ${LIB_TOMPLAYER}/libiniparser.so.0 libiniparser.so.0
ln -sf ${LIB_TOMPLAYER}/libdirect-1.2.so.0.0.0 libdirect-1.2.so.0.0.0
ln -sf ${LIB_TOMPLAYER}/libIL.so.1.0.0 libIL.so.1.0.0
ln -sf ${LIB_TOMPLAYER}/libjpeg.la libjpeg.la
ln -sf ${LIB_TOMPLAYER}/libfusion.la libfusion.la
ln -sf ${LIB_TOMPLAYER}/libILUT.so.1.0.0 libILUT.so.1.0.0
ln -sf ${LIB_TOMPLAYER}/libdirectfb.la libdirectfb.la
ln -sf ${LIB_TOMPLAYER}/libz.so.1.2.3 libz.so.1.2.3
ln -sf ${LIB_TOMPLAYER}/libpng12.so.0.29.0 libpng12.so.0.29.0
ln -sf ${LIB_TOMPLAYER}/libILU.so.1.0.0 libILU.so.1.0.0
ln -sf ${LIB_TOMPLAYER}/libpng12.la libpng12.la
ln -sf ${LIB_TOMPLAYER}/libts.la libts.la
ln -sf ${LIB_TOMPLAYER}/libjpeg.so.62.0.0 libjpeg.so.62.0.0
ln -sf ${LIB_TOMPLAYER}/libzip.so.1.0.0 libzip.so.1.0.0
ln -sf ${LIB_TOMPLAYER}/libpng.so.3.29.0 libpng.so.3.29.0
ln -sf ${LIB_TOMPLAYER}/libdirectfb-1.2.so.0.0.0 libdirectfb-1.2.so.0.0.0
ln -sf ${LIB_TOMPLAYER}/libfreetype.la libfreetype.la
ln -sf ${LIB_TOMPLAYER}/libts-0.0.so.0.1.1 libts.so
ln -sf ${LIB_TOMPLAYER}/libz.so.1.2.3 libz.so.1
ln -sf ${LIB_TOMPLAYER}/libIL.so.1.0.0 libIL.so
ln -sf ${LIB_TOMPLAYER}/libpng12.la libpng.la
ln -sf ${LIB_TOMPLAYER}/libpng12.so.0.29.0 libpng12.so
ln -sf ${LIB_TOMPLAYER}/libfreetype.so.6.3.16 libfreetype.so
ln -sf ${LIB_TOMPLAYER}/libz.so.1.2.3 libz.so
ln -sf ${LIB_TOMPLAYER}/libpng.so.3.29.0 libpng.so.3
ln -sf ${LIB_TOMPLAYER}/libfusion-1.2.so.0.0.0 libfusion-1.2.so.0
ln -sf ${LIB_TOMPLAYER}/libdirect-1.2.so.0.0.0 libdirect-1.2.so.0
ln -sf ${LIB_TOMPLAYER}/libpng12.pc libpng.pc
ln -sf ${LIB_TOMPLAYER}/libILUT.so.1.0.0 libILUT.so
ln -sf ${LIB_TOMPLAYER}/libdirect-1.2.so.0.0.0 libdirect.so
ln -sf ${LIB_TOMPLAYER}/libIL.so.1.0.0 libIL.so.1
ln -sf ${LIB_TOMPLAYER}/libILU.so.1.0.0 libILU.so
ln -sf ${LIB_TOMPLAYER}/libILU.so.1.0.0 libILU.so.1
ln -sf ${LIB_TOMPLAYER}/libdirectfb-1.2.so.0.0.0 libdirectfb.so
ln -sf ${LIB_TOMPLAYER}/libiniparser.so.0 libiniparser.so
ln -sf ${LIB_TOMPLAYER}/libfreetype.so.6.3.16 libfreetype.so.6
ln -sf ${LIB_TOMPLAYER}/libjpeg.so.62.0.0 libjpeg.so
ln -sf ${LIB_TOMPLAYER}/libfusion-1.2.so.0.0.0 libfusion.so
ln -sf ${LIB_TOMPLAYER}/libzip.so.1.0.0 libzip.so
ln -sf ${LIB_TOMPLAYER}/libts-0.0.so.0.1.1 libts-0.0.so.0
ln -sf ${LIB_TOMPLAYER}/libzip.so.1.0.0 libzip.so.1
ln -sf ${LIB_TOMPLAYER}/libdirectfb-1.2.so.0.0.0 libdirectfb-1.2.so.0
ln -sf ${LIB_TOMPLAYER}/libpng12.so libpng.so
ln -sf ${LIB_TOMPLAYER}/libjpeg.so.62.0.0 libjpeg.so.62
ln -sf ${LIB_TOMPLAYER}/libpng12.so.0.29.0 libpng12.so.0
ln -sf ${LIB_TOMPLAYER}/libILUT.so.1.0.0 libILUT.so.1


cd $TOMPLAYER_DIR
./tomplayer --dfb:no-vt

ttn_file=/bin/ttn

ttn_cnt=`ps | grep ttn | wc -l`

if test ${ttn_cnt} -lt 2 ; then 


echo "Start TTN"
/etc/rc.restartgps
${ttn_file}
fi
