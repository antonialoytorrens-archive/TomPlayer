LOG=/dev/null
#LOG=/media/sdcard/tomplayer/testbt.txt

# refresh for some time to be able to log navcore launch 
/media/sdcard/tomplayer/rwdg.sh &


if [ -f /media/sdcard/tomplayer/boot.txt ] ; then

/media/sdcard/tomplayer/bt_tom.sh
/media/sdcard/tomplayer/tomplayergui.sh
TOM_EXIT=$?
if [ $TOM_EXIT -ne 51 ] ; then
rm -f /media/sdcard/tomplayer/boot.txt
sync
fi
killall -9 refresh_wdg

else

start_ttn
/media/sdcard/tomplayer/waitkey.sh &
/media/sdcard/tomplayer/rc.bluetooth

fi

killall -9 rwdg.sh
