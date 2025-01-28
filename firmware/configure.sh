#!/usr/bin/env bash

set -ex

# tiny44a
# debug build
cmake \
    --toolchain tiny44a-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -G Ninja \
    -B build/debug44

# release build
cmake \
    --toolchain tiny44a-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja \
    -B build/release44



# tiny84a
# debug build
cmake \
    --toolchain tiny84a-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -G Ninja \
    -B build/debug84

# release build
cmake \
    --toolchain tiny84a-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja \
    -B build/release84



# x64
# tests
cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -G Ninja \
    -B build/tests-host
