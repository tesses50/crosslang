#!/bin/bash
cmake -S ../.. -B build-x86 -DCMAKE_TOOLCHAIN_FILE=`pwd`/WindowsToolchains/x86.cmake -DTESSESFRAMEWORK_FETCHCONTENT=ON -DTESSESFRAMEWORK_ENABLE_APPS=OFF -DTESSESFRAMEWORK_ENABLE_EXAMPLES=OFF -DTESSESFRAMEWORK_ENABLE_STATIC=ON -DTESSESFRAMEWORK_ENABLE_SHARED=OFF -DCMAKE_INSTALL_PREFIX=tmp-x86
cd build-x86
make -j`nproc`
cpack -G NSIS
cpack -G ZIP
cp TessesCrossLang-*-win32.exe ../../../artifacts/crosslang-win32.exe
cp TessesCrossLang-*-win32.zip ../../../artifacts/crosslang-win32.zip
cd ..
cmake -S ../.. -B build-x64 -DCMAKE_TOOLCHAIN_FILE=`pwd`/WindowsToolchains/x64.cmake -DTESSESFRAMEWORK_FETCHCONTENT=ON -DTESSESFRAMEWORK_ENABLE_APPS=OFF -DTESSESFRAMEWORK_ENABLE_EXAMPLES=OFF -DTESSESFRAMEWORK_ENABLE_STATIC=ON -DTESSESFRAMEWORK_ENABLE_SHARED=OFF -DCMAKE_INSTALL_PREFIX=tmp-x64
cd build-x64
make -j`nproc`
cpack -G NSIS
cpack -G ZIP
cp TessesCrossLang-*-win64.exe ../../../artifacts/crosslang-win64.exe
cp TessesCrossLang-*-win64.zip ../../../artifacts/crosslang-win64.zip
