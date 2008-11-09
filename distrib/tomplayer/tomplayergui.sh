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
export TSLIB_PLUGINDIR=/usr/local/lib/ts

export LIB_TOMPLAYER=${TOMPLAYER_DIR}/lib
export CONF_TOMPLAYER=${TOMPLAYER_DIR}/conf

export LD_LIBRARY_PATH=/usr/local/lib

# Call script that creates symlinks for shared librairies
./create_symlinks.sh

# Copy tslib configuration file
cd /etc
ln -sf ${CONF_TOMPLAYER}/ts.conf ts.conf


cd $TOMPLAYER_DIR

END_ASKED=0
NO_SPLASH=
while [  $END_ASKED -eq 0 ]; do 
rm /tmp/start_engine.sh
./tomplayer --dfb:no-vt $NO_SPLASH
sync
./splash_screen res/background/loading
if [ -f /tmp/start_engine.sh ] ; then 
  /bin/sh /tmp/start_engine.sh
   NO_SPLASH="--no-splash"
else 
  END_ASKED=1
fi
done

ttn_file=/bin/ttn

ttn_cnt=`ps | grep ttn | wc -l`

if test ${ttn_cnt} -lt 2 ; then 


echo "Start TTN"
/etc/rc.restartgps
${ttn_file}
fi
