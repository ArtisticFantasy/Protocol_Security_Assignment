#!/bin/bash
source $(dirname $0)/env_setup.sh
cd ${PROJECT_DIR}
rm -rf build
rm -rf gps
mkdir build
cd build && cmake ${GPS_CPP_DIR} && make -j8 || exit 1
ln -sf ${PROJECT_DIR}/build/bin/main ${PROJECT_DIR}/gps