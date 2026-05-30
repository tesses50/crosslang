#!/bin/bash
mkdir -p ../../artifacts
cmake -S ../.. -B build-x86 -DCMAKE_TOOLCHAIN_FILE=`pwd`/WindowsToolchains/x86.cmake -DTESSESFRAMEWORK_FETCHCONTENT=ON -DTESSESFRAMEWORK_ENABLE_APPS=OFF -DTESSESFRAMEWORK_ENABLE_EXAMPLES=OFF -DTESSESFRAMEWORK_STATIC=ON -DCMAKE_INSTALL_PREFIX=tmp-x86 -DCMAKE_BUILD_TYPE=Release
cd build-x86
make -j`nproc`
cpack -G NSIS
mv TessesCrossLang-*-win32.exe ../../../artifacts/crosslang-win32-setup.exe
mv crosslang.exe ../../../artifacts/crosslang-win32-portable.exe
cd ..
cmake -S ../.. -B build-x64 -DCMAKE_TOOLCHAIN_FILE=`pwd`/WindowsToolchains/x64.cmake -DTESSESFRAMEWORK_FETCHCONTENT=ON -DTESSESFRAMEWORK_ENABLE_APPS=OFF -DTESSESFRAMEWORK_ENABLE_EXAMPLES=OFF -DTESSESFRAMEWORK_STATIC=ON -DCMAKE_INSTALL_PREFIX=tmp-x64 -DCMAKE_BUILD_TYPE=Release
cd build-x64
make -j`nproc`
cpack -G NSIS
mv TessesCrossLang-*-win64.exe ../../../artifacts/crosslang-win64-setup.exe
mv crosslang.exe ../../../artifacts/crosslang-win64-portable.exe
