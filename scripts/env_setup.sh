#!/bin/bash
export PROJECT_DIR=$(realpath $(dirname ${BASH_SOURCE[0]})/..)
export SCRIPT_DIR=${PROJECT_DIR}/scripts
export GPS_CPP_DIR=${PROJECT_DIR}/gps_cpp
export RESULT_DIR=${PROJECT_DIR}/results
export CONFIG_DIR=${PROJECT_DIR}/config

CONFIG_FILE=${CONFIG_DIR}/config.ini
NETWORKS_FILE=${CONFIG_DIR}/networks.txt

SOURCE_IP=""
GATEWAY_MAC=""
SEND_INTERFACE=""
PACKET_RATE=""
WORK_THREADS=""
LZR_BIN=""
SCAN_PERCENTAGE=100
SUBNET_LENGTH=16
LAST_ITERATION=""
ITERATION_NUMBER=1
ACTIVE_IP_FILE=""
USE_SEED_FILE=""

if [ -f "$CONFIG_FILE" ]; then
    while IFS='=' read -r key value || [ -n "$key" ]; do
        if [[ $key != \#* && -n $key ]]; then
            key=$(echo "$key" | xargs)
            value=$(echo "$value" | xargs)
            case "$key" in
                SOURCE_IP)
                    SOURCE_IP="$value"
                    ;;
                GATEWAY_MAC)
                    GATEWAY_MAC="$value"
                    ;;
                SEND_INTERFACE)
                    SEND_INTERFACE="$value"
                    ;;
                PACKET_RATE)
                    PACKET_RATE=$value
                    ;;
                WORK_THREADS)
                    WORK_THREADS=$value
                    ;;
                LZR_BIN)
                    LZR_BIN="$value"
                    ;;
                SCAN_PERCENTAGE)
                    SCAN_PERCENTAGE=$value
                    ;;
                SUBNET_LENGTH)
                    SUBNET_LENGTH=$value
                    ;;
                NETWORKS_FILE)
                    NETWORKS_FILE="$value"
                    ;;
                LAST_ITERATION)
                    LAST_ITERATION="$value"
                    ;;
                ITERATION_NUMBER)
                    ITERATION_NUMBER=$value
                    ;;
                ACTIVE_IP_FILE)
                    ACTIVE_IP_FILE="$value"
                    ;;
                USE_SEED_FILE)
                    USE_SEED_FILE="$value"
                    ;;
                *)
                    echo "Unknown key: $key"
                    exit 1
                    ;;
            esac
        fi
    done < ${CONFIG_FILE}
else
    echo "${CONFIG_FILE} does not exist."
fi

if [ ! -f "$NETWORKS_FILE" ]; then
    echo "${NETWORKS_FILE} does not exist."
    exit 1
fi

if [ ! -z "$ACTIVE_IP_FILE" ] && [ ! -f "$ACTIVE_IP_FILE" ]; then
    echo "${ACTIVE_IP_FILE} does not exist."
    exit 1
fi

if [ ! -z "$USE_SEED_FILE" ] && [ ! -f "$USE_SEED_FILE" ]; then
    echo "${USE_SEED_FILE} does not exist."
    exit 1
fi

if [ -z "$SOURCE_IP" ]; then
    echo "SOURCE_IP is not set in ${CONFIG_FILE}."
    exit 1
fi

if [ -z "$GATEWAY_MAC" ]; then
    echo "GATEWAY_MAC is not set in ${CONFIG_FILE}."
    exit 1
fi

if [ -z "$SEND_INTERFACE" ]; then
    echo "SEND_INTERFACE is not set in ${CONFIG_FILE}."
    exit 1
fi

if [ -z "$PACKET_RATE" ]; then
    echo "PACKET_RATE is not set in ${CONFIG_FILE}."
    exit 1
fi

if [ -z "$WORK_THREADS" ]; then
    echo "WORK_THREADS is not set in ${CONFIG_FILE}."
    exit 1
fi

if [ -z "$LZR_BIN" ]; then
    echo "LZR_BIN is not set in ${CONFIG_FILE}."
    exit 1
fi

if ! [[ $PACKET_RATE =~ ^[0-9]+$ ]]; then
    echo "PACKET_RATE is not a valid integer."
    exit 1
fi

if ! [[ $WORK_THREADS =~ ^[0-9]+$ ]]; then
    echo "WORK_THREADS is not a valid integer."
    exit 1
fi

if ! [[ $SCAN_PERCENTAGE =~ ^[0-9]*\.?[0-9]+$ ]]; then
    echo "SCAN_PERCENTAGE is not a valid float number."
    exit 1
fi

if ! [[ $SUBNET_LENGTH =~ ^[0-9]+$ ]]; then
    echo "SUBNET_LENGTH is not a valid integer."
    exit 1
fi

if ! [[ $ITERATION_NUMBER =~ ^[0-9]+$ ]]; then
    echo "ITERATION_NUMBER is not a valid integer."
    exit 1
fi

if [ -z "$LAST_ITERATION" ]; then
    LAST_ITERATION_DIR=""
else
    LAST_ITERATION_DIR=${RESULT_DIR}/${LAST_ITERATION}
    if [ ! -d "$LAST_ITERATION_DIR" ]; then
        echo "LAST_ITERATION_DIR does not exist."
        exit 1
    fi
fi

export SOURCE_IP=${SOURCE_IP}
export GATEWAY_MAC=${GATEWAY_MAC}
export SEND_INTERFACE=${SEND_INTERFACE}
export PACKET_RATE=${PACKET_RATE}
export WORK_THREADS=${WORK_THREADS}
export LZR_BIN=${LZR_BIN}
export SCAN_PERCENTAGE=${SCAN_PERCENTAGE}
export SUBNET_LENGTH=${SUBNET_LENGTH}