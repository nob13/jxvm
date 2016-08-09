#!/bin/bash
set -e
MYDIR=`dirname $0`
cd $MYDIR

if [ -z "${JAVA_HOME}" ]; then
	echo "JAVA_HOME not set"
	exit 1
fi

# Building java lib
cd javalib
ant jar
ant test_jar
cd ..

# Building CPP Code
mkdir -p build
cd build
cmake .. # this also copys the Jars created above

make -j4

# Execute Testcases
./test/jx_test


