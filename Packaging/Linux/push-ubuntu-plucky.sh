#!/bin/bash
source ./version.sh
upload() {
    curl --user tesses50:$GITEA_AUTH -X DELETE \
    https://git.tesseslanguage.com/api/packages/tesses50/debian/pool/plucky/main/crosslang/$DEB_VERSION/$1
    curl --user tesses50:$GITEA_AUTH \
    --upload-file build/plucky/crosslang_$DEB_VERSION\_$1\.deb \
    https://git.tesseslanguage.com/api/packages/tesses50/debian/pool/plucky/main/upload
}
upload amd64
upload arm64
upload riscv64
upload i386
