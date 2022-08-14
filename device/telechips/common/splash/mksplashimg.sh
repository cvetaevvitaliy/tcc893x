# !bin/sh


function ussage() {
cat <<EOT
Ussage: mkspalsh pagesize img_fmt img_num imgfile1 imgfile2 .... outfile
	pagesize = if you use MTD for splash partition intput physical nand page size the default size is 512
		   it have to specified!!
	img_fmt = format of image files for 16 bit or 32bit checking
	img_num = number of image files for generate to splash image
	imgfile = the image files that you want to add, the number of file have to equal to img_num
	outfile = target out splash image file.
EOT
}


echo "MAKE SPLASH IMGAE ........"

PARAMS=($*)
LENGTH=${#PARAMS[*]}
PAGESIZE=${PARAMS[0]}
FMTSIZE=${PARAMS[1]}
FILENAME=${PARAMS[$LENGTH-1]}
NIMG=${PARAMS[2]}

#if [ $# -lt "4" -o $# -lt "$NIMG" ]; then
if [ $# -lt "4" -o $# -lt "$NIMG" ]; then
ussage
#exit
fi

if [ "$PAGESIZE" -lt "512" ] ; then 
echo "ERROR : [ default pagesize is 512 ] "
ussage
#exit
fi


for ((idx=0; idx<$NIMG; idx++));
do
#IMG_ORI[$idx]=${PARAMS[$idx+2]}
IMG_ORI[$idx]=${PARAMS[$idx+3]}
IMG_EXT[$idx]=${IMG_ORI[$idx]%%.*}.tmp
IMG_FN[$idx]=${IMG_ORI[$idx]%%.*}.img

IMG_RSL[$idx]=$(identify "${IMG_ORI[$idx]}" | cut -f 3 -d' ')

IMG_BITS[$idx]=$(file "${IMG_ORI[$idx]}" | cut -f 11 -d' ')


if [ "$FMTSIZE" -eq "32" ];  then
convert -depth 8 ${IMG_ORI[$idx]} rgb:${IMG_EXT[$idx]}
rgbto888 <${IMG_EXT[$idx]}> ${IMG_FN[$idx]}

else if [ "$FMTSIZE" -eq "16" ] ; then

convert -depth 8 ${IMG_ORI[$idx]} rgb:${IMG_EXT[$idx]}
rgb2565 <${IMG_EXT[$idx]}> ${IMG_FN[$idx]}

else
echo "ERROR : [ choose fmt 16 or 32] "
ussage
fi
fi

done

echo ${IMG_RSL[*]}
echo ${IMG_BITS[*]}

MK_SPLASH="mksplash $PAGESIZE $NIMG ${IMG_FN[*]} ${IMG_RSL[*]} $FILENAME"
echo $MK_SPLASH
$MK_SPLASH


