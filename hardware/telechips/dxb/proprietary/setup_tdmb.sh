#!/bin/sh
echo "standards is"  $standards"(TDMB)"
echo "Please select broadcasting baseband type"
echo "1.TCC3150"
echo "2.TCC351X"
echo "3.TCC3161"

read -p "Select : " baseband    
if [ "$baseband" == "1" ]
then 
    #set prpperty to TCC3150
    echo "baseband is"  $baseband"(TCC3150)"
    setprop tcc.dxb.tdmb.baseband 1
elif [ "$baseband" == "2" ]
then
    #set prpperty to TCC351X 
    echo "baseband is"  $baseband"(TCC351X)"
    setprop tcc.dxb.tdmb.baseband 2  
elif [ "$baseband" == "3" ]
then
    #set prpperty to TCC3161 
    echo "baseband is"  $baseband"(TCC3161)"
    setprop tcc.dxb.tdmb.baseband 3
fi
