mkdir -p ../../artifacts
mkdir -p builds
git clone --depth 1 https://git.tesses.org/tesses50/zig-cross builds/zig-cross



for tripple in x86_64-linux-musl x86-linux-musl aarch64-linux-musl arm-linux-musleabi riscv64-linux-musl powerpc-linux-musleabihf; do
    export BUILDDIR=builds/$tripple
    mkdir -p $BUILDDIR
    cmake -S ../.. -B $BUILDDIR --toolchain $PWD/builds/zig-cross/$tripple\.cmake -DCMAKE_BUILD_TYPE=Release -DTESSESFRAMEWORK_ENABLE_EXAMPLES=OFF -DTESSESFRAMEWORK_ENABLE_SHARED=OFF -DTESSESFRAMEWORK_ENABLE_STATIC=ON -DCROSSLANG_ENABLE_SUPERSLIM=ON -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++ -static -Wl,--strip-all" -DCMAKE_POSITION_INDEPENDENT_CODE=ON -GNinja
    cmake --build $BUILDDIR || exit 1
    cmake --install $BUILDDIR --prefix $BUILDDIR/out
    mv $BUILDDIR/out/bin/crosslang ../../artifacts/crosslang-slim-$tripple
done

for tripple in x86_64-windows-gnu x86-windows-gnu aarch64-windows-gnu; do
    export BUILDDIR=builds/$tripple
    mkdir -p $BUILDDIR
    cmake -S ../.. -B $BUILDDIR --toolchain $PWD/builds/zig-cross/$tripple\.cmake -DCMAKE_BUILD_TYPE=Release  -DTESSESFRAMEWORK_ENABLE_EXAMPLES=OFF -DTESSESFRAMEWORK_ENABLE_SHARED=OFF -DTESSESFRAMEWORK_ENABLE_STATIC=ON -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++ -static -Wl,--strip-all" -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCROSSLANG_ENABLE_SUPERSLIM=ON -GNinja
    cmake --build $BUILDDIR || exit 1
    cmake --install $BUILDDIR --prefix $BUILDDIR/out

    mv $BUILDDIR/out/bin/crosslang.exe ../../artifacts/crosslang-slim-$tripple\.exe
done

for tripple in x86_64-macos-none aarch64-macos-none; do
    export BUILDDIR=builds/$tripple
    mkdir -p $BUILDDIR
    cmake -S ../.. -B $BUILDDIR --toolchain $PWD/builds/zig-cross/$tripple\.cmake -DCMAKE_BUILD_TYPE=Release -DTESSESFRAMEWORK_ENABLE_EXAMPLES=OFF -DTESSESFRAMEWORK_ENABLE_SHARED=OFF -DTESSESFRAMEWORK_ENABLE_STATIC=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCROSSLANG_ENABLE_SUPERSLIM=ON -GNinja -DCMAKE_EXE_LINKER_FLAGS="-Wl,--strip-all"
    cmake --build $BUILDDIR || exit 1
    cmake --install $BUILDDIR --prefix $BUILDDIR/out

    mv $BUILDDIR/out/bin/crosslang ../../artifacts/crosslang-slim-$tripple
done