# !/bin/sh

# for using quick boot branch merging

#change current dir to root dir
cd ../../../

# setup build environment 
. build/envsetup.sh
lunch

#repo forall -c git reset --hard jb-mr1-beta-release-20130218
#repo forall -c git reset --hard
#repo sync

# branch merging 
echo "Start Mergingg jellybean-mr1 and Quic Boot Branch"
cd $ANDROID_BUILD_TOP

#cd vendor/telechips/tools/
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git pull telechips jb-mr1.1
#cd $ANDROID_BUILD_TOP

#cd bootable/bootloader/lk
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git branch -D quickboot_jb
#git branch quickboot_jb
#git checkout quickboot_jb
#git pull telechips quickboot_jb
#cd $ANDROID_BUILD_TOP

#cd device/telechips/tcc8920
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git branch -D quickboot_jb
#git branch quickboot_jb
#git checkout quickboot_jb
#git pull telechips quickboot_jb
#cd $ANDROID_BUILD_TOP

#cd device/telechips/tcc8920st
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git branch -D quickboot_jb
#git branch quickboot_jb
#git checkout quickboot_jb
#git pull telechips quickboot_jb
#cd $ANDROID_BUILD_TOP

#cd device/telechips/tcc8930st
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git branch -D quickboot_jb
#git branch quickboot_jb
#git checkout quickboot_jb
#git pull telechips quickboot_jb
#cd $ANDROID_BUILD_TOP

#cd device/telechips/tcc893x
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git branch -D quickboot_jb
#git branch quickboot_jb
#git checkout quickboot_jb
#git pull telechips quickboot_jb
#cd $ANDROID_BUILD_TOP

cd device/telechips/tcc892x-common
echo "$PWD ------------------------------------------------------------------------------------------------------" 
git branch -D quickboot_jb
git branch quickboot_jb
git checkout quickboot_jb
git pull telechips quickboot_jb
cd $ANDROID_BUILD_TOP

cd device/telechips/tcc893x-common
echo "$PWD ------------------------------------------------------------------------------------------------------" 
git branch -D quickboot_jb
git branch quickboot_jb
git checkout quickboot_jb
git pull telechips quickboot_jb
cd $ANDROID_BUILD_TOP

#cd device/telechips/common
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git branch -D quickboot_jb_lib
#git branch quickboot_jb_lib
#git checkout quickboot_jb_lib
#git pull telechips quickboot_jb_lib
#cd $ANDROID_BUILD_TOP

#cd frameworks/base
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git branch jb-mr1.1
#git checkout jb-mr1.1
#git reset --hard jb-mr1-beta-release-20130218

#git branch -D quickboot_jb_lib
#git branch quickboot_jb_lib
#git checkout quickboot_jb_lib
#git reset --hard jb-mr1-beta-release-20130218
#git pull telechips quickboot_jb_lib

#git checkout jb-mr1.1
#git branch -D quickboot_jb_proguard
#git branch quickboot_jb_proguard
#git checkout quickboot_jb_proguard
#git reset --hard jb-mr1-beta-release-20130218
#git pull telechips quickboot_jb_proguard

#git checkout jb-mr1.1
#git branch -D quickboot_jb
#git branch quickboot_jb
#git checkout quickboot_jb
#git reset --hard jb-mr1-beta-release-20130218
#git pull telechips quickboot_jb

#cd $ANDROID_BUILD_TOP

#cd bionic
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git branch -D jb-mr1.1
#git branch jb-mr1.1
#git checkout jb-mr1.1
#git pull telechips quickboot_jb
#cd $ANDROID_BUILD_TOP

#cd system/core
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git branch -D quickboot_jb
#git branch quickboot_jb
#git checkout quickboot_jb
#git pull telechips quickboot_jb
#cd $ANDROID_BUILD_TOP

cd packages/apps/Settings
echo "$PWD ------------------------------------------------------------------------------------------------------" 
git branch -D quickboot_jb
git branch quickboot_jb
git checkout quickboot_jb
git pull telechips quickboot_jb
cd $ANDROID_BUILD_TOP

#cd bootable/recovery
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git branch -D quickboot_jb
#git branch quickboot_jb
#git checkout quickboot_jb
#git pull telechips quickboot_jb
#cd $ANDROID_BUILD_TOP

cd build
echo "$PWD ------------------------------------------------------------------------------------------------------"
git branch -D quickboot_jb
git branch quickboot_jb
git checkout quickboot_jb
git pull telechips quickboot_jb
cd $ANDROID_BUILD_TOP

#cd kernel
#echo "$PWD ------------------------------------------------------------------------------------------------------" 
#git branch -D quickboot_jb
#git branch quickboot_jb
#git checkout quickboot_jb
#git pull telechips quickboot_jb
#cd $ANDROID_BUILD_TOP

rm $ANDROID_PRODUCT_OUT/system/build.prop

. build/envsetup.sh
lunch


