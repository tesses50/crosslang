#!/bin/bash
find publish -name "*.crvm" -exec upload-package --token="$CPKG_KEY" --host="https://cpkg.tesseslanguage.com/" "{}" \;