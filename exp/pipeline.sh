source $(realpath $(dirname $0)/..)/scripts/env_setup.sh
echo "-----------Config Statistics---------"
echo "PROJECT_DIR: ${PROJECT_DIR}"
echo "SCRIPT_DIR: ${SCRIPT_DIR}"
echo "RESULT_DIR: ${RESULT_DIR}"
echo "CONFIG_FILE: ${CONFIG_FILE}"
echo "NETWORKS_FILE: ${NETWORKS_FILE}"
echo "SOURCE_IP: ${SOURCE_IP}"
echo "SEND_INTERFACE: ${SEND_INTERFACE}"
echo "GATEWAY_MAC: ${GATEWAY_MAC}"
echo "PACKET_RATE: ${PACKET_RATE}"
echo "WORK_THREADS: ${WORK_THREADS}"
echo "LZR_BIN: ${LZR_BIN}"
echo "SCAN_PERCENTAGE: ${SCAN_PERCENTAGE}"
echo "SUBNET_LENGTH: ${SUBNET_LENGTH}"
echo "ITERATION_NUMBER: ${ITERATION_NUMBER}"
if [ -z ${LAST_ITERATION} ]; then
    echo "LAST_ITERATION: None"
else
    echo "LAST_ITERATION: ${LAST_ITERATION}"
fi
if [ -z ${USE_SEED_FILE} ] && [ ! -z ${ACTIVE_IP_FILE} ]; then
    echo "ACTIVE_IP_FILE: ${ACTIVE_IP_FILE}"
fi
if [ ! -z ${USE_SEED_FILE} ]; then
    echo "USE_SEED_FILE: ${USE_SEED_FILE}"
fi
echo "-------------------------------------"

cd ${PROJECT_DIR}

bash ${SCRIPT_DIR}/drop_rst.sh || exit 1

# Build GPS
echo "---------------Build GPS--------------"
bash ${SCRIPT_DIR}/build.sh || exit 1
echo "--------------Finish Build------------"
for i in $(seq 1 ${ITERATION_NUMBER}); do
    echo "=============Start Iteration ${i}=============="
    RATIO=1.0
    CUR_RESULT_DIR=${RESULT_DIR}/$(date +%Y%m%d-%H:%M)
    rm -rf ${CUR_RESULT_DIR}
    mkdir -p ${CUR_RESULT_DIR}
    echo "Setting up current result directory: ${CUR_RESULT_DIR}"

    if [ ! -z ${LAST_ITERATION_DIR} ]; then
        if [ ! -f ${LAST_ITERATION_DIR}/final.json ]; then
            echo "LAST_ITERATION_DIR does not contain final.json."
            exit 1
        fi
        touch ${CUR_RESULT_DIR}/last_iter.txt
        echo "Last iteration: ${LAST_ITERATION_DIR}" > ${CUR_RESULT_DIR}/last_iter.txt
    else
        touch ${CUR_RESULT_DIR}/network_span.txt
        echo "Network file: ${NETWORKS_FILE}" > ${CUR_RESULT_DIR}/network_span.txt
    fi

    # Pre Scan
    if [ -z ${LAST_ITERATION_DIR} ] && [ -z ${USE_SEED_FILE} ]; then
        echo "---------------Pre Scan---------------"
        ACTIVE_IP_LIST=${CUR_RESULT_DIR}/active_ip_list.txt
        if [ ! -z  ${ACTIVE_IP_FILE} ]; then
            cp ${ACTIVE_IP_FILE} ${ACTIVE_IP_LIST}
            touch ${CUR_RESULT_DIR}/use_active_ip.txt
            echo "Using active IP list from ${ACTIVE_IP_FILE}." > ${CUR_RESULT_DIR}/use_active_ip.txt
            echo "Using active IP list from ${ACTIVE_IP_FILE}."
        else
            python ${SCRIPT_DIR}/pre_scan.py -o ${ACTIVE_IP_LIST} -n ${NETWORKS_FILE} \
                -i ${SEND_INTERFACE} -s ${SOURCE_IP} -g ${GATEWAY_MAC} \
                -w ${WORK_THREADS} -r ${PACKET_RATE} -p ${SCAN_PERCENTAGE} || exit 1
            echo "Writing active IP list to ${ACTIVE_IP_LIST}."
        fi
        echo "The length of active IP list is $(wc -l < ${ACTIVE_IP_LIST})."
        echo "Please input the ratio of seed list to active IP list (default 1.0):"
        read RATIO
        if [ -z "$RATIO" ]; then
            RATIO=1.0
        fi
        echo "The ratio of seed list to active IP list is set to ${RATIO}."
        echo "-----------Finish Pre Scan------------"
    fi

    # Seed Scan
    echo "---------------Seed Scan--------------"
    SEED_LIST=${CUR_RESULT_DIR}/seed_list.txt
    SEED_FILE=${CUR_RESULT_DIR}/seed.json
    if [ -z ${LAST_ITERATION_DIR} ]; then
        if [ ! -z ${USE_SEED_FILE} ]; then
            cp ${USE_SEED_FILE} ${SEED_FILE}
            touch ${CUR_RESULT_DIR}/use_seed.txt
            echo "Using seed file from ${USE_SEED_FILE}." > ${CUR_RESULT_DIR}/use_seed.txt
            echo "Using seed file from ${USE_SEED_FILE}."
        else
            python ${SCRIPT_DIR}/get_seed_list.py -i ${ACTIVE_IP_LIST} -o ${SEED_LIST} -r ${RATIO} || exit 1
            bash ${SCRIPT_DIR}/lzr_scan.sh -i ${SEED_LIST} -o ${SEED_FILE} || exit 1
            echo "Writing seed scan results to ${SEED_FILE}."
        fi
    else
        EXTRA_SEED_FILE=${CUR_RESULT_DIR}/extra_seed.json
        python ${SCRIPT_DIR}/get_seed_list_based_on_last_iter.py -i ${LAST_ITERATION_DIR}/final.json -o ${SEED_LIST} || exit 1
        bash ${SCRIPT_DIR}/lzr_scan.sh -i ${SEED_LIST} -o ${EXTRA_SEED_FILE} || exit 1
        python ${SCRIPT_DIR}/combine_results.py ${SEED_FILE} ${LAST_ITERATION_DIR}/final.json ${EXTRA_SEED_FILE} || exit 1
        echo "Writing seed scan results to ${SEED_FILE}."
    fi
    echo "-----------Finish Seed Scan-----------"

    # GPS Part One
    echo "--------------GPS Part One------------"
    SCANNING_PLAN_ONE=${CUR_RESULT_DIR}/scanning_plan_one.txt
    PRED_PATTERN_FILE=${CUR_RESULT_DIR}/pred_pattern.json
    bash ${SCRIPT_DIR}/gps_part_one.sh -i ${SEED_FILE} -o ${SCANNING_PLAN_ONE} -p ${PRED_PATTERN_FILE} -l ${SUBNET_LENGTH} || exit 1
    echo "Writing scanning plan one to ${SCANNING_PLAN_ONE}."
    echo "Writing prediction pattern to ${PRED_PATTERN_FILE}."
    echo "-----------Finish GPS Part One--------"

    # Prior Scan
    echo "--------------Prior Scan--------------"
    PRIOR_FILE=${CUR_RESULT_DIR}/prior.json
    python ${SCRIPT_DIR}/prior_scan.py -i ${SCANNING_PLAN_ONE} -o ${PRIOR_FILE} \
        -d ${SEND_INTERFACE} -s ${SOURCE_IP} -g ${GATEWAY_MAC} \
        -r ${PACKET_RATE} -w ${WORK_THREADS} -l ${LZR_BIN} || exit 1
    echo "Writing prior scan results to ${PRIOR_FILE}."
    echo "-----------Finish Prior Scan-----------"

    # GPS Part Two
    echo "--------------GPS Part Two------------"
    SCANNING_PLAN_TWO=${CUR_RESULT_DIR}/scanning_plan_two.txt
    bash ${SCRIPT_DIR}/gps_part_two.sh -i ${PRIOR_FILE} -o ${SCANNING_PLAN_TWO} -p ${PRED_PATTERN_FILE} -l ${SUBNET_LENGTH} || exit 1
    echo "Writing scanning plan two to ${SCANNING_PLAN_TWO}."
    echo "-----------Finish GPS Part Two--------"

    # Final Scan
    echo "--------------Post Scan--------------"
    POST_FILE=${CUR_RESULT_DIR}/post.json
    bash ${SCRIPT_DIR}/lzr_scan.sh -i ${SCANNING_PLAN_TWO} -o ${POST_FILE} || exit 1
    echo "Writing post scan results to ${POST_FILE}."
    echo "-----------Finish Post Scan-----------"

    # Combine Results
    echo "------------Combine Results-----------"
    FINAL_FILE=${CUR_RESULT_DIR}/final.json
    python ${SCRIPT_DIR}/combine_results.py ${FINAL_FILE} ${SEED_FILE} ${PRIOR_FILE} ${POST_FILE} || exit 1
    echo "Writing final results to ${FINAL_FILE}."
    echo "---------Finish Combine Results-------"
    echo "Finish processing. Results are saved in ${CUR_RESULT_DIR}."
    LAST_ITERATION_DIR=${CUR_RESULT_DIR}
    echo "============Finish Iteration ${i}=============="
done