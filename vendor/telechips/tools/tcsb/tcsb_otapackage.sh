# !bin/sh


function ussage(){
cat<<EOT
Ussage: mkupdate_package.sh -j [jobs]
example : makeupdate_package.sh -j 12
	Before using this secure boot update package making script
	You have to build android system and update otapackage
	and making Secure boot image sboot.img, srecovery.img in product out directory using signing.sh

EOT
}

OUTDIR=$OUT

SBOOTFILE=$OUT/sboot.img
SRECOVERYFILE=$OUT/srecovery.img
SECUREDIR=$OUT/secureboot

OTAPACKAGE=$ANDROID_BUILD_TOP/build/tools/releasetools/ota_from_target_files
BINARY_PATH=$ANDROID_BUILD_TOP/out/host/linux-x86
TESTKEY=$ANDROID_BUILD_TOP/build/target/product/security/testkey

	
if [ "$1" == "-j" ]; then
 JOBS="-j$2"
else
 JOBS=
fi

cd $ANDROID_BUILD_TOP
OTAPACKAGE_CMD="make $JOBS otapackage "
$OTAPACKAGE_CMD

BASEDIR=$OUTDIR/obj/PACKAGING/target_files_intermediates
BASEPACKAGE=($(find $BASEDIR -name $TARGET_PRODUCT*.zip))

if [ ! -e $BASEDIR ]; then
 ussage
 exit 3
fi

if [ ! -e $SBOOTFILE -a ! -e $SRECOVERYFILE ]; then
 ussage
 exit 1
fi

if [ ! -e $SECUREDIR ]; then
 mkdir $SECUREDIR
fi

cp -f "$SBOOTFILE" "$SECUREDIR/boot.img"
cp -f "$SRECOVERYFILE" "$SECUREDIR/recovery.img"


ZIP_CMD="zip -j $OUTDIR/secureimg.zip $SECUREDIR/boot.img $SECUREDIR/recovery.img"
MAKE_UPDATE_PACKAGE=" $OTAPACKAGE -v -p $BINARY_PATH -k $TESTKEY -z False $BASEPACKAGE+$OUTDIR/secureimg.zip $OUTDIR/$TARGET_PRODUCT-ota-tcsb-$TARGET_BUILD_VARIANT-$USER.zip "

echo $ZIP_CMD
$ZIP_CMD
echo $MAKE_UPDATE_PACKAGE
$MAKE_UPDATE_PACKAGE

rm -rf $OUTDIR/secureimg.zip $SECUREDIR


