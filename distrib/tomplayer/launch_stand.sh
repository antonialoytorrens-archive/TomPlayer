/media/sdcard/tomplayer/rwdg.sh &
/media/sdcard/tomplayer/bt_tom.sh
/media/sdcard/tomplayer/tomplayergui.sh
TOM_EXIT=$?
if [ $TOM_EXIT -ne 51 ] ; then
rm -f /media/sdcard/tomplayer/boot.txt
sync
fi
killall -9 refresh_wdg
killall -9 rwdg.sh
