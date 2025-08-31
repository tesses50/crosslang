#!/bin/bash
mkdir -p build
cd build
mkdir build-deb-amd64
cmake -S ../../../ -B build-deb-amd64 -DCMAKE_INSTALL_PREFIX=/usr -DCROSSLANG_FETCHCONTENT=OFF -DCROSSLANG_ENABLE_FFI=ON
cd build-deb-amd64
make -j`nproc`
make install DESTDIR=../crosslang_1.0.0_amd64
mkdir -p ../crosslang_1.0.0_amd64/DEBIAN
mkdir -p ../crosslang_1.0.0_amd64/usr/share/Tesses/CrossLang
wget -O ../crosslang_1.0.0_amd64/usr/share/Tesses/CrossLang/Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm https://downloads.tesses.net/ShellPackage.crvm
cp ../../debian/control-amd64 ../crosslang_1.0.0_amd64/DEBIAN/control
cd ../
dpkg-deb --build crosslang_1.0.0_amd64