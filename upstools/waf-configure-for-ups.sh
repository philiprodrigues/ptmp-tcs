#!/bin/bash

mydir="$(dirname $BASH_SOURCE)"
topdir="$(dirname $mydir)"
echo mydir is $mydir
echo topdir is $topdir
if [ ! -f "$topdir/waftools/waf" ] ; then
    echo "This script assumes it is in the ptmp-tcs source directory under upstools/"
    exit 1
fi

if [ -z "$ZMQ_VERSION" -o -z "$CZMQ_VERSION" -o -z "$PROTOBUF_VERSION" -o -z "$PTMP_VERSION" ] ; then
    cat <<EOF
This script assumes you have your UPS environment set up for the 'ptmp' product and its dependencies ('zmq', 'czmq' and 'protobuf').

Maybe try:

source /cvmfs/fermilab.opensciencegrid.org/products/artdaq/setup
PRODUCTS=$PRODUCTS:/cvmfs/fermilab.opensciencegrid.org/products/larsoft
setup ptmp v0_0_3 -q e15

EOF
    exit 1
fi

prefix="$1" 
if [ -z "$prefix" ] ; then
    prefix="$topdir/install"
fi
echo "installing to $prefix"


${topdir}/waftools/waf configure \
      --with-libzmq-lib=$ZMQ_LIB --with-libzmq-include=$ZMQ_INC \
      --with-libczmq-lib=$CZMQ_LIB --with-libczmq-include=$CZMQ_INC \
      --with-protobuf=$PROTOBUF_FQ_DIR \
      --with-ptmp-lib=$PTMP_LIB --with-ptmp-include=$PTMP_INC \
      --prefix=$prefix || exit 1

echo "Now, you can run './waf install'"
