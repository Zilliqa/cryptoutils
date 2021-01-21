# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(include_dir ${PROJECT_SOURCE_DIR}/include)

cmake_policy(SET CMP0076 NEW)
target_sources(CryptoUtils PUBLIC
    endianness.hpp
    ${include_dir}/ethash/ethash.h
    ${include_dir}/ethash/ethash.hpp
    ethash-internal.hpp
    ethash.cpp
    ${include_dir}/ethash/hash_types.h
    managed.cpp
    ${include_dir}/ethash/keccak.h
    ${include_dir}/ethash/keccak.hpp
    keccak.c
    keccakf1600.c
    primes.h
    primes.c
)
