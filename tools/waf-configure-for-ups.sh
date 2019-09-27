#!/bin/bash

if [ ! -f wscript ] ; then
    echo "This script must be run from the top level source directory"
    exit -1
fi

if [ -z "$ZEROMQ_VERSION" -o -z "$CZMQ_VERSION" -o -z "$PROTOBUF_VERSION" ] ; then
    cat <<EOF
This script assumes you have your UPS environment set up for the products 'zeromq', 'czmq' and 'protobuf'.

Maybe try:

source /cvmfs/fermilab.opensciencegrid.org/products/artdaq/setup
PRODUCTS=$PRODUCTS:/cvmfs/fermilab.opensciencegrid.org/products/larsoft
setup czmq v4_2_0 -q e15
setup protobuf v3_3_1a -q e15

IF the ZeroMQ product names have been fixed, please update this script.

EOF
    exit 1
fi

prefix="$1" 
if [ -z "$prefix" ] ; then
    prefix="$(pwd)/install"
fi
echo "installing to $prefix"

args="--with-protobuf=$PROTOBUF_FQ_DIR"
args="$args --with-libzmq-lib=$ZEROMQ_LIB --with-libzmq-include=$ZEROMQ_INC"
args="$args --with-libczmq-lib=$CZMQ_LIB --with-libczmq-include=$CZMQ_INC"
if [ -n "$PTMP_INC" -a -n "$PTMP_LIB" ] ; then
    echo "Using PTMP include and lib from environment"
    args="$args --with-ptmp-include=$PTMP_INC --with-ptmp-lib=$PTMP_LIB"
elif [ -d pdt ] ; then
    echo 
    echo "Warning: looks like this is ptmp-tcs but no sign of PTMP in UPS variables"
    echo "I guess this will fail"
    echo 
fi

set -x
./tools/waf configure $args --prefix=$prefix || exit 1
set +x

echo "Now maybe run './tools/waf --notests install', etc"
