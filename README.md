# beagle-config

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```
### Current Status
* Before:
[![asciicast](https://asciinema.org/a/427430.svg)](https://asciinema.org/a/427430)
* After
[![asciicast](https://asciinema.org/a/427432.svg)](https://asciinema.org/a/427432)

* Implemented Stuff
[![asciicast](https://asciinema.org/a/427729.svg)](https://asciinema.org/a/427729)

### For Arm

* Either add the following (with your choice of compiler) to `board/CMakeLists.txt` 
```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabi-g++-8)
set(CMAKE_C_COMPILER /usr/bin/arm-linux-gnueabi-gcc-8)

set(CMAKE_FIND_ROOT_PATH  /usr/arm-linux-gnueabihf)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```
* Or
```bash
cd build
export CMAKE_SYSTEM_NAME=Linux
export CMAKE_SYSTEM_PROCESSOR=arm
export CMAKE_C_COMPILER=$(which arm-linux-gnueabi-gcc-8)
export CMAKE_CXX_COMPILER=$(which arm-linux-gnueabi-g++-8)
rm -f CMakeCache.txt
cmake ..   -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}\
            -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}\
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}\
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}\
            -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER\
            -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY\
            -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY
make -j$(nproc)
```

On the board
```sh
sudo ln -s /lib/ld-linux-armhf.so.3 /lib/ld-linux.so.3
./beagle-config
```
