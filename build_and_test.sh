#!/bin/bash
MODE=RELEASE
BUILD_DIR=Release
cmake -E make_directory ${BUILD_DIR} && cmake -E chdir ${BUILD_DIR} cmake .. -DCMAKE_BUILD_TYPE=${MODE} && cmake --build ${BUILD_DIR} -- -j2 && ${BUILD_DIR}/ecsps_test
