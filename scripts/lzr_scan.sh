#!/bin/sh
source $(dirname $0)/env_setup.sh

while getopts "i:o:" opt; do
  case $opt in
    i) input_file="$OPTARG" ;;
    o) output_file="$OPTARG" ;;
    *) echo "Invalid option: -$OPTARG" >&2; exit 1 ;;
  esac
done

> "$output_file"

cat "$input_file" | pv -L${PACKET_RATE} -l --quiet | sudo ${LZR_BIN} -w ${WORK_THREADS} -t 3 -sendSYNs \
-onlyDataRecord -sendInterface ${SEND_INTERFACE} -sourceIP ${SOURCE_IP} -gatewayMac ${GATEWAY_MAC} \
--handshakes ftp,http,ssh,telnet,tls,vnc,mongodb,mqtt,mssql,mysql,oracle,pop3,postgres,pptp,rdp,redis,rtsp,amqp,dnp3,dns,fox,imap,ipmi,ipp,kubernetes,memcached_ascii,memcached_binary,modbus,newlines,newlines50,siemens,smb,smtp,wait,x11 \
-f "$output_file"