# !bin/sh

function ussage(){
cat<<EOT
Ussage:
Signing.sh --type	<emmc | nand_v7 | nand_v8>
	   [--pagesize	<nand page size>] only for use  nand_v7 type
	
emmc type : SD / T-Flash / MMC / EMMC
nand_v7 type : Pure NAND Flash Memory - NAND Driver Ver.7
nand_v8 type : Pure NAND Flash Memory - NAND Driver Ver.8
pagesize : for alignment of pure NAND flash memory page size. 
EOT
}

#echo "IN signing.sh PATH=$PATH"

#######################################
# KEY Definition 
#######################################
DEF_KEYDIR=$ANDROID_BUILD_TOP/vendor/telechips/tools/tcsb/sign_key
SIGN=sign
TYPE=BLw
SIGNALGO=ecdsa224
HASHALGO=sha224
PRIKEYFILE=$DEF_KEYDIR/tcsb_keypair.der
WRAPKEYFILE=$DEF_KEYDIR/tcsb_sbcr.bin
#######################################

if [ ! -e $DEF_KEYDIR ]; then 
 echo " there is not exist the $DEF_KEYDIR dir"
 echo " please create default key store directory and key files for signing"
 exit 4
fi

if [ ! -f $PRIKEYFILE -o ! -f $WRAPKEYFILE ]; then 
 echo " there is missing key files check key store !! "
 exit 7
fi



if [ $# -ne 2 -a $# -ne 4 ]; then
 ussage
 exit 1
fi

TARGET_OUT=$OUT

BOOTFILE=$TARGET_OUT/boot.img
RECOVERYFILE=$TARGET_OUT/recovery.img
TEMP_SIGNED_IMG=$TARGET_OUT/stemp.img

SBOOTIMG=$TARGET_OUT/sboot.img
SRECOVERYIMG=$TARGET_OUT/srecovery.img

if [ ! -f $BOOTFILE -o ! -f $RECOVERYFILE ]; then 
 echo " there is not exist boot.img or recovery.img in $TARGET_OUT dir !!"
 echo " before using this signing script,  please build android system first !!"
exit 10
fi

if [ "$2" != "emmc" -a "$2" != "nand_v7" -a "$2" != "nand_v8" ]; then
 echo "do not suppoer such [$2] typs!!"
 exit 3
fi

STORAGE_TYPE=$2

if [ "$STORAGE_TYPE" != "emmc" ]; then
 if [ "$STORAGE_TYPE" != "nand_v8" ]; then
  if [ -z $3 ]; then
   echo " this [$STORAGE_TYPE] type need pagesizes "
   ussage
   exit 5
  fi
  PAGESIZE=$4
 else
  if [ ! -z $3 ]; then
   echo " this [$STORAGE_TYPE] type does not need pagesizes "
   exit 6
  fi
  PAGESIZE=
 fi 

else
 if [ ! -z $3 ]; then
  echo " this [$STORAGE_TYPE] type does not need pagesizes "
  exit 6
 fi
 PAGESIZE=
fi

MAKE_BOOT_SIGNED_CMD="tcsb_signtool $SIGN -type $TYPE -signalgo $SIGNALGO -hashalgo $HASHALGO
					-prikey $PRIKEYFILE -wrapkey $WRAPKEYFILE -infile $BOOTFILE -outfile $TEMP_SIGNED_IMG"

MAKE_RECOVERY_SIGNED_CMD="tcsb_signtool $SIGN -type $TYPE -signalgo $SIGNALGO -hashalgo $HASHALGO
					-prikey $PRIKEYFILE -wrapkey $WRAPKEYFILE -infile $RECOVERYFILE -outfile $TEMP_SIGNED_IMG"

MAKE_SBOOT_CMD="tcsb_mkimg $TEMP_SIGNED_IMG $SBOOTIMG $PAGESIZE"

MAKE_SRECOVERY_CMD="tcsb_mkimg $TEMP_SIGNED_IMG $SRECOVERYIMG $PAGESIZE"

$MAKE_BOOT_SIGNED_CMD
echo $MAKE_BOOT_SIGNED_CMD
$MAKE_SBOOT_CMD
echo $MAKE_SBOOT_CMD
rm -rf $TEMP_SIGNED_IMG

$MAKE_RECOVERY_SIGNED_CMD
echo $MAKE_RECOVERY_SIGNED_CMD
$MAKE_SRECOVERY_CMD
echo $MAKE_SRECOVERY_CMD
rm -rf $TEMP_SIGNED_IMG
