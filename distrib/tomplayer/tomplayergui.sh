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

echo "Running the application"

#Create a symbolic link to enable generic minigui configuration file
ln -sf ${TOMPLAYER_DIR}/res /etc/res

#convert in UNIX text format the used configuration file
cp -f tomplayer.ini /tmp/tomplayer.ini
dos2unix /tmp/tomplayer.ini

export FRAMEBUFFER=/dev/fb
./tomplayer

ttn_file=/bin/ttn

ttn_cnt=`ps | grep ttn | wc -l`

if test ${ttn_cnt} -lt 2 ; then 


echo "Start TTN"
/etc/rc.restartgps
${ttn_file}
fi
