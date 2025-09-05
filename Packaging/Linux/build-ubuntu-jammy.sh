#!/bin/bash
source ./version.sh
mkdir -p build/jammy
cd build/jammy
wget -O Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm https://downloads.tesses.net/ShellPackage.crvm
mkdir build-amd64
apt install -y tessesframework
cmake -S ../../../../ -B build-amd64 -DCMAKE_INSTALL_PREFIX=/usr -DCROSSLANG_FETCHCONTENT=OFF -DCROSSLANG_ENABLE_FFI=ON
cd build-amd64
make -j`nproc`
make install DESTDIR=../crosslang_$DEB_VERSION\_amd64
apt purge -y tessesframework
mkdir -p ../crosslang_$DEB_VERSION\_amd64/DEBIAN
bash ../../../make-control.sh ../crosslang_$DEB_VERSION\_amd64/DEBIAN/control amd64
cd ../
mkdir -p crosslang_$DEB_VERSION\_amd64/usr/share/Tesses/CrossLang
cp Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm crosslang_$DEB_VERSION\_amd64/usr/share/Tesses/CrossLang/Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm
dpkg-deb --build crosslang_$DEB_VERSION\_amd64

foreign() {
    apt install -y tessesframework:$1
    mkdir build-$1
    cmake -S ../../../../ -B build-$1 -DCMAKE_INSTALL_PREFIX=/usr -DCROSSLANG_FETCHCONTENT=OFF -DCROSSLANG_ENABLE_FFI=ON -DCMAKE_TOOLCHAIN_FILE=/opt/toolchains/$1\.cmake
    cd build-$1
    make -j`nproc`
    make install DESTDIR=../crosslang_$DEB_VERSION\_$1
    mkdir -p ../crosslang_$DEB_VERSION\_$1/DEBIAN
    bash ../../../make-control.sh ../crosslang_$DEB_VERSION\_$1\/DEBIAN/control $1
    cd ../
    mkdir -p crosslang_$DEB_VERSION\_$1\/usr/share/Tesses/CrossLang
    cp Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm crosslang_$DEB_VERSION\_$1\/usr/share/Tesses/CrossLang/Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm
    dpkg-deb --build crosslang_$DEB_VERSION\_$1
    apt purge -y tessesframework:$1
}

foreign arm64
foreign armhf
foreign riscv64

