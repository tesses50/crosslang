#!/bin/bash
source ./version.sh
upload() {
    curl --user tesses50:$GITEA_AUTH -X DELETE \
    https://git.tesseslanguage.com/api/packages/tesses50/debian/pool/jammy/main/crosslang/$DEB_VERSION/$1
    curl --user tesses50:$GITEA_AUTH \
    --upload-file build/jammy/crosslang_$DEB_VERSION\_$1\.deb \
    https://git.tesseslanguage.com/api/packages/tesses50/debian/pool/jammy/main/upload
}
upload amd64
upload arm64
upload riscv64

