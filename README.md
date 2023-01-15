<br />
<p align="center">
  <a href="https://github.com/SAtacker/bb-config">
    <img src="assets/images/beaglecfg.png" alt="Logo" width="481" height="200">
  </a>
  <p align="center">
    Configure your beagle devices easily.
    <br/>
    <br/>
    <a href="https://github.com/SAtacker/bb-config/wiki"><strong>Explore the docs »</strong></a>
    <br />
    <a href="https://github.com/SAtacker/bb-config/issues">Report Bug</a>
    ·
    <a href="https://github.com/SAtacker/bb-config/wiki/Examples">Examples</a>
    .
    <a href="https://github.com/SAtacker/bb-config/issues">Request Feature</a>
    ·
    <a href="https://github.com/SAtacker/bb-config/pulls">Send a Pull Request</a>
  </p>
</p>

<p align="center">
  <img src="https://github.com/SAtacker/bb-config/actions/workflows/armhf-release.yml/badge.svg">
  <img src="https://github.com/SAtacker/bb-config/actions/workflows/pull_request_build.yml/badge.svg?branch=main" >
  <img src="https://img.shields.io/github/stars/SAtacker/bb-config">
  <img src="https://img.shields.io/github/forks/SAtacker/bb-config">
  <img src="https://img.shields.io/github/issues/SAtacker/bb-config">
  <img src="https://img.shields.io/github/repo-size/SAtacker/bb-config">
  <img src="https://img.shields.io/github/license/SAtacker/bb-config">
</p>

## Build

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```
### Looks like
![gif](assets/beaglecfg.gif)

* Note: For Cross Compiling a script `cross_compile.sh` should work fine on debian based distros.
