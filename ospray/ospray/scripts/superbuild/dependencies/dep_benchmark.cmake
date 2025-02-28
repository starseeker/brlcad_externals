## Copyright 2021 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

set(COMPONENT_NAME benchmark)

if (INSTALL_IN_SEPARATE_DIRECTORIES)
  set(COMPONENT_PATH ${INSTALL_DIR_ABSOLUTE}/${COMPONENT_NAME})
else()
  set(COMPONENT_PATH ${INSTALL_DIR_ABSOLUTE})
endif()

ExternalProject_Add(${COMPONENT_NAME}
  URL "https://github.com/google/benchmark/archive/refs/tags/v1.6.1.zip"
  URL_HASH "SHA256=367e963b8620080aff8c831e24751852cffd1f74ea40f25d9cc1b667a9dd5e45"

  # Skip updating on subsequent builds (faster)
  UPDATE_COMMAND ""

  DEPENDS gtest

  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX:PATH=${COMPONENT_PATH}
    -DBENCHMARK_ENABLE_TESTING=OFF
    -DBENCHMARK_ENABLE_GTEST_TESTS=OFF
    -DBENCHMARK_ENABLE_WERROR=OFF
    -DCMAKE_BUILD_TYPE=${DEPENDENCIES_BUILD_TYPE}
)

list(APPEND CMAKE_PREFIX_PATH ${COMPONENT_PATH})
string(REPLACE ";" "|" CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")
