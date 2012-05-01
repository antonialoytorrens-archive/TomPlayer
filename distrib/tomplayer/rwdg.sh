i=0
while [ $i -lt 60 ] ; do
#let i=i+1
sleep 1
echo -n "*" > /dev/watchdog
done
