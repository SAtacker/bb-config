# beagle-config

```bash
mkdir -p build || cd build
cmake ..
make -j8
```

### Current Status
[![asciicast](https://asciinema.org/a/QhBR4ByTSmMg9Pl4j38LuoCh9.svg)](https://asciinema.org/a/QhBR4ByTSmMg9Pl4j38LuoCh9)

### For Arm

* Make sure to remove the `CMakeCache.txt` after initiaing cmake once

In `build` directory

```sh
rm CMakeCache.txt
cmake -DBUILD_ARM=ON ..  
make --jobs=8
```


On the board
```sh
sudo ln -s /lib/ld-linux-armhf.so.3 /lib/ld-linux.so.3
./beagle-config
```

[![asciicast](https://asciinema.org/a/QkZqUxBLt5kj3iyKz1ToKwedj.svg)](https://asciinema.org/a/QkZqUxBLt5kj3iyKz1ToKwedj)