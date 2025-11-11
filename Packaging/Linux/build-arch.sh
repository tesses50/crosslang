#!/bin/bash
cd /home/build
mkdir x86_64
cd x86_64
cp ../PKGBUILD .
makepkg
curl --user tesses50:$GITEA_AUTH \
     --upload-file *.pkg.tar.zst \
     https://git.tesseslanguage.com/api/packages/tesses50/arch/core
cd ..
mkdir powerpc
cd powerpc
cp ../PKGBUILD .
CARCH=powerpc CMAKE_TOOLCHAIN=/opt/cross/ppc/toolchain.cmake makepkg
curl --user tesses50:$GITEA_AUTH \
     --upload-file *.pkg.tar.zst \
     https://git.tesseslanguage.com/api/packages/tesses50/arch/core
