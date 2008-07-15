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

mkdir /usr/lib
cd /usr/lib
ln -sf ${LIB_TOMPLAYER}/directfb-1.2-0 directfb-1.2-0
ln -sf ${LIB_TOMPLAYER}/ts ts

ln -sf ${LIB_TOMPLAYER}/libfusion-1.2.so.0.0.0 libfusion-1.2.so.0
ln -sf ${LIB_TOMPLAYER}/libzip.so.1.0.0 libzip.so.1
ln -sf ${LIB_TOMPLAYER}/libdirect-1.2.so.0.0.0 libdirect-1.2.so.0
ln -sf ${LIB_TOMPLAYER}/libz.so.1.2.3 libz.so.1
ln -sf ${LIB_TOMPLAYER}/libpng12.so.0 libpng12.so.0
ln -sf ${LIB_TOMPLAYER}/libdirectfb-1.2.so.0.0.0 libdirectfb-1.2.so.0
ln -sf ${LIB_TOMPLAYER}/libfreetype.so.6.3.16 libfreetype.so.6
ln -sf ${LIB_TOMPLAYER}/libpng.so.3.29.0 libpng.so.3
ln -sf ${LIB_TOMPLAYER}/libts-0.0.so.0.1.1 libts-0.0.so.0
ln -sf ${LIB_TOMPLAYER}/libIL.so.1.0.0 libIL.so.1
ln -sf ${LIB_TOMPLAYER}/libILU.so.1.0.0 libILU.so.1
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
