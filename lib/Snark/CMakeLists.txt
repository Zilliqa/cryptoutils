# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

cmake_policy(SET CMP0076 NEW)
target_sources(CryptoUtils PUBLIC
    Snark.cpp
)
