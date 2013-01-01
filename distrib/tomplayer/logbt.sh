while [ 1 ] ; do
  hciconfig
  /media/sdcard/tomplayer/hcitool dev | grep hci0
  HCIOK=$?
  if [ $HCIOK -eq 0 ] ; then
    /media/sdcard/tomplayer/hcidump -X -V  hidp 
  fi
  sleep 5
  let RETRY=RETRY+1
done
