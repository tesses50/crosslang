echo "Building Windows Static x86"
cmake -S "ConsoleOrServer" -B build/win-x86  -DCROSSAPPPUBLISH_STATIC=ON -DCMAKE_TOOLCHAIN_FILE=`pwd`/WindowsToolchains/x86.cmake
cd build/win-x86
make -j`nproc`
cd ../..
echo "Building Windows Static x64"
cmake -S "ConsoleOrServer" -B build/win-x64  -DCROSSAPPPUBLISH_STATIC=ON -DCMAKE_TOOLCHAIN_FILE=`pwd`/WindowsToolchains/x64.cmake
cd build/win-x64
make -j`nproc`
cd ../..