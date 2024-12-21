#!/bin/bash
source $(dirname $0)/env_setup.sh
sudo iptables -C OUTPUT -p tcp --tcp-flags RST RST -s ${SOURCE_IP} -j DROP > /dev/null 2>&1

if [ $? -ne 0 ]; then
    sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -s ${SOURCE_IP} -j DROP
    echo "Dropped RST packets from ${SOURCE_IP}"
fi