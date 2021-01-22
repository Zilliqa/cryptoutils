# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

target_sources(CryptoUtils PUBLIC
    Snark.cpp
)

target_link_libraries(CryptoUtils ff)
