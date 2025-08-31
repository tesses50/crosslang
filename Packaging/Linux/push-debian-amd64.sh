#!/bin/bash
curl --user tesses50:$GITEA_AUTH -X DELETE \
     https://git.tesseslanguage.com/api/packages/tesses50/debian/pool/jammy/main/crosslang/1.0.0/amd64
curl --user tesses50:$GITEA_AUTH \
    --upload-file build/crosslang_1.0.0_amd64.deb \
    https://git.tesseslanguage.com/api/packages/tesses50/debian/pool/jammy/main/upload
