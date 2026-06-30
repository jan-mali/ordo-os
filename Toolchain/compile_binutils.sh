#!/bin/bash

set -e

cd $PREFIX/..
mkdir -p binutils-build 

tar -xzvf binutils-2.45.tar.gz

cd binutils-build
../binutils-2.45/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j $(nproc)
make install
