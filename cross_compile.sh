#!/usr/bin/bash
set -e

check_and_install(){
    for var in "$@"
    do
        PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $var|grep "install ok installed")
        echo Checking for $var: $PKG_OK
        if [ "" = "$PKG_OK" ]; then
            echo "No $var. Setting up $var."
            sudo apt-get --yes install $var 
        fi
    done
}

check_and_install "gcc-8-arm-linux-gnueabihf" "g++-8-arm-linux-gnueabihf" "cmake"
mkdir -p build
cd build
cmake ..    -DCMAKE_CXX_COMPILER=$(which arm-linux-gnueabihf-g++-8) \
            -DCMAKE_C_COMPILER=$(which arm-linux-gnueabihf-gcc-8) \
            -DVERSION_FILE=ON
make -j$(nproc)
