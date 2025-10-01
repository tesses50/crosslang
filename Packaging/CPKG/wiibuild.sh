#!/bin/bash
mkdir -p publish/wii
cd Wii
mkdir build
cd build
cmake -S .. -B . -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/Wii.cmake
make -j`nproc`
mv boot.dol ../../publish/wii/boot.dol