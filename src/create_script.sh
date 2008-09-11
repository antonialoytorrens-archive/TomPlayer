# This script
#   - Copy dependencies 
#   - Generate a script to create symlinks on target fs
#

TOMPLAYER_FOLDER=../distrib/tomplayer/
DISTRIB_TREE=deps
DEP_DEST_FOLDER=$TOMPLAYER_FOLDER/$DISTRIB_TREE

RPATH=/usr/local
REF_TREE=./dependencies/build/
REF_FOLDER_LIST="lib etc"

OUTPUT_FILENAME=create_symlinks.sh

INIT_DIR=`pwd`
 
rm $OUTPUT_FILENAME
echo "LOCAL_DIR=\`pwd\`" > $OUTPUT_FILENAME
echo "mkdir -p $RPATH" >> $OUTPUT_FILENAME

# copying necessary files and creating script for symlinks
for FOLDER in $REF_FOLDER_LIST ; do 
cd $REF_TREE/$RPATH/$FOLDER
LIST=`find . -type d`
echo $LIST | awk -v path=$INIT_DIR/$TOMPLAYER_FOLDER/$DISTRIB_TREE/$RPATH/$FOLDER/ '{for (i=1;i<=NF;i++) {command="mkdir -p " path "/"$i;  system(command)}}'
echo $LIST | awk -v path=$RPATH/$FOLDER/ '{for (i=1;i<=NF;i++) {command="mkdir -p " path "/"$i; print command ;}}'  >> $INIT_DIR/$OUTPUT_FILENAME

LIST=`find . -type f`
echo $LIST | awk -v path=$INIT_DIR/$TOMPLAYER_FOLDER/$DISTRIB_TREE/$RPATH/$FOLDER/  '{for (i=1;i<=NF;i++) {command="cp " $i " " path "/" $i;  system(command)}}'
echo $LIST | awk -v rpath=$RPATH/$FOLDER -v dest=$DISTRIB_TREE/$RPATH/$FOLDER '{for (i=1;i<=NF;i++) {print "ln -sf  $LOCAL_DIR/" dest "/" $i " " rpath "/" $i }}' >> $INIT_DIR/$OUTPUT_FILENAME

cd $INIT_DIR
done




# Generating script for symlinks
for FOLDER in $REF_FOLDER_LIST ; do 
cd $REF_TREE/$RPATH/$FOLDER
find . -type l | xargs ls -l | grep "\->" |  awk -v rpath=$RPATH/$FOLDER -v dest=$DISTRIB_TREE/$RPATH/$FOLDER '{  i=split($9, field , "/") ; base=field[i] ; print "ln -sf $LOCAL_DIR/"  dest "/" $11 " " rpath "/" base}' >> $INIT_DIR/$OUTPUT_FILENAME
cd $INIT_DIR
done


mv $OUTPUT_FILENAME $TOMPLAYER_FOLDER