#!/usr/bin/env bash

mkdir -p build
cmake -S . -B build/ -DWITH_PROCPS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=1
cmake --build build/
