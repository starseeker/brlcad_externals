# Unless we have ENABLE_ALL set, based the building of libregex on the system
# detection results
if (ENABLE_ALL)
  set(ENABLE_REGEX ON)
endif (ENABLE_ALL)

if (NOT ENABLE_REGEX)

  find_package(REGEX)

  if (NOT REGEX_FOUND AND NOT DEFINED ENABLE_REGEX)
    set(ENABLE_REGEX "ON" CACHE BOOL "Enable regex build")
  endif (NOT REGEX_FOUND AND NOT DEFINED ENABLE_REGEX)

endif (NOT ENABLE_REGEX)
set(ENABLE_REGEX "${ENABLE_REGEX}" CACHE BOOL "Enable regex build")

if (ENABLE_REGEX)

  set(REGEX_PREFIX_STR "libregex_")

  # Platform differences in default linker behavior make it difficult to
  # guarantee that our libregex symbols will override libc. We'll avoid the
  # issue by renaming our libregex symbols to be incompatible with libc.
  ExternalProject_Add(REGEX_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/regex"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    $<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}>
    -DBIN_DIR=${BIN_DIR}
    -DLIB_DIR=${LIB_DIR}
    -DBUILD_STATIC_LIBS=${BUILD_STATIC_LIBS}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DREGEX_PREFIX_STR=${REGEX_PREFIX_STR}
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    )

  SetTargetFolder(REGEX_BLD "Third Party Libraries")
  SetTargetFolder(regex "Third Party Libraries")

endif (ENABLE_REGEX)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

