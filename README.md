# beagle-config

```bash
mkdir -p build || cd build
cmake ..
make -j8
```

### Current Status
[![asciicast](https://asciinema.org/a/XPxOSDxAXEvBEK5efNpVU6bEe.svg)](https://asciinema.org/a/XPxOSDxAXEvBEK5efNpVU6bEe)

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

[![asciicast](https://asciinema.org/a/i0CJ56NdL6e28jXl31RQuRYUl.svg)](https://asciinema.org/a/i0CJ56NdL6e28jXl31RQuRYUl)