if [ $# -lt 2 ]
then
    echo "Not enough args, pass nfpm arch and debian tripple as argument"
    echo "This script requires our docker container: git.tesses.org/tesses50/linux-*:latest"
else
    curl https://git.tesses.org/tesses50/tessesframework/releases/download/v$TF_VERSION/tessesframework-v$TF_VERSION-$1.tar.gz | tar xz -C /opt/cross/staging
    mkdir -p build
    cd build
    cmake-cross -S .. -B . -DCROSSLANG_NFPM_ARCH=$1 -DCMAKE_INSTALL_LIBDIR=lib/$2 -DCMAKE_BUILD_TYPE=Release -DCROSSLANG_FETCHCONTENT=OFF -DCROSSLANG_TF_VERSION=$TF_VERSION
    make -j`nproc`
    make install DESTDIR=debian
    cd debian
    nfpm package -f ../nfpm.yaml -p deb
    curl --user tesses50:$PACKAGE_AND_BREW \
     --upload-file *.deb \
     https://git.tesses.org/api/packages/tesses50/debian/pool/universal/main/upload
    #debian/pool/bionic
    cd ..
    if [ "$1" = "386" ] || [ "$1" = "arm7" ]
    then
    cmake-cross -S .. -B . -DCROSSLANG_NFPM_ARCH=$1 -DCMAKE_INSTALL_LIBDIR=lib32
    else
    cmake-cross -S .. -B . -DCROSSLANG_NFPM_ARCH=$1 -DCMAKE_INSTALL_LIBDIR=lib64
    fi
    make install DESTDIR=fedora
    cd fedora
    nfpm package -f ../nfpm.yaml -p rpm
    curl --user tesses50:$PACKAGE_AND_BREW \
     --upload-file *.rpm \
     https://git.tesses.org/api/packages/tesses50/rpm/upload
    cd ..
    cmake-cross -S .. -B . -DCROSSLANG_NFPM_ARCH=$1 -DCMAKE_INSTALL_LIBDIR=lib
    make install DESTDIR=arch
    cd arch
    nfpm package -f ../nfpm.yaml -p archlinux
    curl --user tesses50:$PACKAGE_AND_BREW \
     --upload-file *.pkg.tar.zst \
     https://git.tesses.org/api/packages/tesses50/arch/core
    mkdir -p ../../artifacts
    tar czf ../../artifacts/crosslang-$VERSION\-$1\.tar.gz --owner=1000 --group=1000 -C usr/ . -C /opt/cross/staging/ .
fi