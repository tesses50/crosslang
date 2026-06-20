curl https://git.tesses.org/tesses50/tessesframework/releases/download/v$TF_VERSION/tessesframework-v$TF_VERSION-powerpc.tar.gz | tar xz -C /opt/cross/staging
    
mkdir -p build
pushd build
cmake-cross -S .. -B . -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_BUILD_TYPE=Release
make -j`nproc`
mkdir -p /home/build/cl
chown build:build -R /home/build
make install DESTDIR=/home/build/cl/src
cp ppc-pkgbuild /home/build/cl/PKGBUILD
chmod 644 /home/build/cl/PKGBUILD
chown build:build -R /home/build/cl
pushd /home/build/cl
su build -c 'CARCH=powerpc makepkg -d'
curl --user tesses50:$PACKAGE_AND_BREW \
     --upload-file *.pkg.tar.zst \
     https://git.tesses.org/api/packages/tesses50/arch/core
popd
popd
mkdir -p artifacts
tar --owner=1000 --group=1000 czf artifacts/crosslang-$VERSION\-powerpc.tar.gz -C /home/build/cl/src/usr . -C /opt/cross/staging/ .