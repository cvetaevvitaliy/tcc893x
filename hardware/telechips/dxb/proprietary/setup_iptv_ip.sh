#!/bin/sh
echo "TCC IPTV Set Server IP & PORT"  
echo "Please select IPTV Server IP"
read -p "Select ip: " ip    
setprop tcc.iptv.set.ip $ip
echo "Please select IPTV PORT"
read -p "Select port: " port    
setprop tcc.iptv.set.port $port

