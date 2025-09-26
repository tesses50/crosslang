#!/bin/bash
rm -r compilers
mkdir -p compilers
cd compilers
wget https://downloads.tesses.net/cache/compilers/$1-cross.tgz
tar xvzf $1-cross.tgz
mv $1-cross/* .
rmdir $1-cross
rm $1-cross.tgz