#!/bin/sh

BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="${BASE_DIR}/build"

trap "" SIGABRT

for test in `ls ${BUILD_DIR}`
do
	${BUILD_DIR}/${test}
	if [ $? -ne 0 ]; then
		echo "Test ${test} Failed (code $?)"
	fi
done

