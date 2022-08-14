#!/bin/sh

echo "This is setup shell for broadcasting"
echo "Please select broadcasting standards"
echo "1.TDMB"
echo "2.DVBT"
echo "3.IPTV"

read -p "Select : " standards

if [ "$standards" == "1" ]
then
    busybox sh /system/bin/setup_tdmb.sh
fi

if [ "$standards" == "2" ]
then
    busybox sh /system/bin/setup_dvbt.sh
fi

if [ "$standards" == "3" ]
then
    busybox sh /system/bin/setup_iptv.sh
fi
