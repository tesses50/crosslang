#!/bin/bash
mkdir -p ../../artifacts
mkdir -p crosslang-$TOOLCHAIN\-temp
cd crosslang-$TOOLCHAIN\-temp
export DIR="$PWD"
echo "set(CMAKE_C_COMPILER \"$DIR/$TOOLCHAIN\-cross/bin/$TOOLCHAIN\-gcc\")" > toolchain.cmake
echo "set(CMAKE_CXX_COMPILER \"$DIR/$TOOLCHAIN\-cross/bin/$TOOLCHAIN\-g++\")" >> toolchain.cmake
echo "set(CMAKE_C_FLAGS \"-static-libgcc -static-libstdc++ -static\")" >> toolchain.cmake
echo "set(CMAKE_CXX_FLAGS \"-static-libgcc -static-libstdc++ -static\")" >> toolchain.cmake
echo "set(CMAKE_EXE_LINKER_FLAGS \"-static-libgcc -static-libstdc++ -static\")" >> toolchain.cmake
#echo "set(CMAKE_SYSROOT \"$DIR/sysroot\")" >> toolchain.cmake
echo "set(CMAKE_SYSTEM_NAME Linux)" >> toolchain.cmake
echo "set(CMAKE_SYSTEM_PROCESSOR $ARCH)" >> toolchain.cmake
echo "set(CMAKE_POSITION_INDEPENDENT_CODE ON)" >> toolchain.cmake
wget https://downloads.tesses.net/cache/compilers/$TOOLCHAIN\-cross.tgz
#wget https://downloads.tesses.net/cache/libraries/source/mbedtls-3.6.3.1.tar.bz2
#wget -O tf.tar.gz "https://onedev.site.tesses.net/~downloads/projects/1/archives?revision=master&format=tgz"
tar xvzf $TOOLCHAIN\-cross.tgz
mkdir build
cd build
cmake -S ../../../../ -B . -DTESSESFRAMEWORK_ENABLE_SHARED=OFF -DTESSESFRAMEWORK_FETCHCONTENT=ON -DTESSESFRAMEWORK_ENABLE_STATIC=ON -DCMAKE_TOOLCHAIN_FILE="$DIR/toolchain.cmake" -DCROSSLANG_ENABLE_SUPERSLIM=ON -DCMAKE_BUILD_TYPE=Release
make -j`nproc`
cp crosslang ../../../../artifacts/crosslang-slim-$TOOLCHAIN
mkdir -p ../../../../builds/containers/$ARCH/usr/bin
cp crosslang ../../../../builds/containers/$ARCH/usr/bin/crosslang
ln -s crosslang ../../../../builds/containers/$ARCH/usr/bin/crossint
ln -s crosslang ../../../../builds/containers/$ARCH/usr/bin/crossc
ln -s crosslang ../../../../builds/containers/$ARCH/usr/bin/crossvm
ln -s crosslang ../../../../builds/containers/$ARCH/usr/bin/crossarchivecreate
ln -s crosslang ../../../../builds/containers/$ARCH/usr/bin/crossarchiveextract
ln -s crosslang ../../../../builds/containers/$ARCH/usr/bin/crossdump
rm -rf "$DIR"
