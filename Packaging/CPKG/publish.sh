#!/bin/bash
find publish -name "*.crvm" -exec crosslang upload-package --token="$CPKG_KEY" --host="https://cpkg.tesseslanguage.com/" "{}" \;