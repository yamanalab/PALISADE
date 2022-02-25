#!/bin/bash

set -eu

rm -rf build
cmake -S . -B build \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=yes \
	-DCMAKE_INSTALL_PREFIX=$HOME/.local \
	-DWITH_INTEL_HEXL=ON \
	-DINTEL_HEXL_PREBUILT=ON
cmake --build build --target testall -j
cmake --install build
