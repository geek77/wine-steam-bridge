#!/bin/sh
PREFIX=i586-mingw32msvc
export CC=$PREFIX-gcc
export CXX=$PREFIX-g++
export CPP=$PREFIX-cpp
export RANLIB=$PREFIX-ranlib
export PATH="/usr/$PREFIX/bin:$PATH"
$CC $@