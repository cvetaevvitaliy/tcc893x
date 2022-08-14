#!/bin/sh
echo "standards is (DVBT)"
echo "Please select broadcasting baseband type"
echo "1.Bibcom9090M"
echo "2.TCC351X_CSPI_STS"
echo "3.TCC351X_I2C_STS"
read -p "Select : " baseband    
setprop tcc.dxb.dvbt.number_of_baseband 1
if [ "$baseband" == "1" ]
then 
    #set prpperty to dibcom
    echo "baseband is"  $baseband"(Dibcom9090M)"
    setprop tcc.dxb.dvbt.baseband 1
elif [ "$baseband" == "2" ]
then
    #set prpperty to TCC351X_CSPI_STS
    echo "baseband is"  $baseband"(TCC351X_CSPI_STS)"
    setprop tcc.dxb.dvbt.baseband 2  
elif [ "$baseband" == "3" ]
then
    #set prpperty to TCC351X_I2C_STS
    echo "baseband is"  $baseband"(TCC351X_I2C_STS)"
    setprop tcc.dxb.dvbt.baseband 3
    echo "Please select the number of basebands[1~4]"
    read -p "Select[1~4] : " number_of_baseband   
    setprop tcc.dxb.dvbt.number_of_baseband $number_of_baseband
    echo "the number of basebands is "$number_of_baseband
fi
