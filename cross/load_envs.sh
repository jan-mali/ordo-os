echo This file should be sourced, not run directly!

echo PWD=$PWD
export PREFIX="$PWD/prefix"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

echo PREFIX=$PREFIX
