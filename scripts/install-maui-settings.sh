#!/bin/bash

mkdir -p maui-settings && cd maui-settings
cmake -DCMAKE_BUILD_TYPE="Release" \
    -DCMAKE_INSTALL_PREFIX="/usr" \
    ../../

make -j$(nproc)
make install