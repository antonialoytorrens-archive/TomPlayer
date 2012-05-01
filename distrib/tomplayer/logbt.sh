while [ 1 ] ; do
HCIOK=1
RETRY=0
while [ $HCIOK -a $RETRY -lt 10 ] ; do
/media/sdcard/tomplayer/hcitool dev | grep hci0
HCIOK=$?
if [ $HCIOK -eq 0 ] ; then
/media/sdcard/tomplayer/hcidump -X -V  hidp 
fi
sleep 1
let RETRY=RETRY+1
done
/etc/rc.bluetooth suspend
/etc/rc.bluetooth resume
sleep 2
done 
