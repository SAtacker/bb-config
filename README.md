<br />
<p align="center">
  <a href="https://github.com/SAtacker/beagle-config">
    <img src="assets/images/beaglecfg.png" alt="Logo" width="481" height="75">
  </a>
  <p align="center">
    Configure your beagle devices easily.
    <br/>
    <br/>
    <a href="https://github.com/SAtacker/beagle-config/wiki"><strong>Explore the docs »</strong></a>
    <br />
    <a href="https://github.com/SAtacker/beagle-config/issues">Report Bug</a>
    ·
    <a href="https://github.com/SAtacker/beagle-config/wiki/Examples">Examples</a>
    .
    <a href="https://github.com/SAtacker/beagle-config/issues">Request Feature</a>
    ·
    <a href="https://github.com/SAtacker/beagle-config/pulls">Send a Pull Request</a>
  </p>
</p>

<p align="center">
  <img src="https://github.com/SAtacker/beagle-config/actions/workflows/armhf-build.yml/badge.svg">
  <img src="https://img.shields.io/github/stars/SAtacker/beagle-config">
  <img src="https://img.shields.io/github/forks/SAtacker/beagle-config">
  <img src="https://img.shields.io/github/issues/SAtacker/beagle-config">
  <img src="https://img.shields.io/github/repo-size/SAtacker/beagle-config">
  <img src="https://img.shields.io/github/license/SAtacker/beagle-config">
</p>

## Build

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```
### Looks like
[![asciicast](https://asciinema.org/a/429873.svg)](https://asciinema.org/a/429873)

* Note: For Cross Compiling a script `cross_compile.sh` should work fine on debian based distros.
