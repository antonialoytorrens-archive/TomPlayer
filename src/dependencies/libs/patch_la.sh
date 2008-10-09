#
# Patch libdir in la files for cross compilation needs
#


function use { 
  echo "$0  prefix abs_install_dir"
  exit 1
}



if [ $# -ne 2 ] ; then
  use
fi


for L in `ls -1 $2/$1/*.la`; do 

cat $L |  awk -v s=$1 -v r=$2/$1 '{ gsub(" " s ," " r, $0); print }' > tmp
mv tmp $L

done

#grep -v libdir $1 > tmp.la
#echo "libdir=$2" >> tmp.la
#mv tmp.la  $1
