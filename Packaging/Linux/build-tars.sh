#!/bin/bash
mkdir build-x86_64-tar
cd build-x86_64-tar
cmake -S ../../.. -B . -DTESSESFRAMEWORK_ENABLE_SHARED=ON -DTESSESFRAMEWORK_ENABLE_STATIC=OFF -DTESSESFRAMEWORK_FETCHCONTENT=ON
make -j`nproc`
make install DESTDIR=./crosslang
mkdir -p crosslang/share/Tesses/CrossLang
cp ../build/jammy/Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm crosslang/share/Tesses/CrossLang/Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm
tar cvzf ../../../artifacts/crosslang-linux-x86_64.tar.gz crosslang
cd ..

foreign() {
mkdir build-$1\-tar
cd build-$1\-tar
cmake -S ../../.. -B . -DTESSESFRAMEWORK_ENABLE_SHARED=ON -DTESSESFRAMEWORK_ENABLE_STATIC=OFF -DTESSESFRAMEWORK_FETCHCONTENT=ON -DCMAKE_TOOLCHAIN_FILE=/opt/toolchains/$1\.cmake
make -j`nproc`
make install DESTDIR=./crosslang
mkdir -p crosslang/share/Tesses/CrossLang
cp ../build/jammy/Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm crosslang/share/Tesses/CrossLang/Tesses.CrossLang.ShellPackage-1.0.0.0-prod.crvm
tar cvzf ../../../artifacts/crosslang-linux-$1\.tar.gz crosslang
cd ..
}
foreign arm64
foreign riscv64

