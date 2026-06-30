#!/bin/bash

set -e

which -- $TARGET-as || echo $TARGET-as is not in the PATH

cd $PREFIX/..
mkdir -p gcc-build 

tar -xzvf gcc-15.2.0.tar.gz

cd gcc-build
../gcc-15.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx CXXFLAGS="-g -O2 -fno-char8_t"

MAKEOPTS="-j `nproc`"

make $MAKEOPTS all-gcc
make $MAKEOPTS all-target-libgcc
make $MAKEOPTS all-target-libstdc++-v3

make install-gcc
make install-target-libgcc
make install-target-libstdc++-v3
