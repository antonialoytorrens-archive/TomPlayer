#Wait for a press on BACK key
/media/sdcard/tomplayer/wait_key
echo "If this file exists, the boot will launch tomplayer otherwise navcore will directly start" > /media/sdcard/tomplayer/boot.txt
sync
# Kill ttn 
killall -9 ttn
