# Unless we have ENABLE_ALL set, based the building of png on
# the system detection results
if (ENABLE_ALL AND NOT DEFINED ENABLE_PNG)
  set(ENABLE_PNG ON)
endif (ENABLE_ALL AND NOT DEFINED ENABLE_PNG)

if (NOT ENABLE_PNG)

  # We generally don't want the Mac framework libpng...
  set(CMAKE_FIND_FRAMEWORK LAST)

  find_package(PNG)

  if (NOT PNG_FOUND AND NOT DEFINED ENABLE_PNG)
    set(ENABLE_PNG "ON" CACHE BOOL "Enable png build")
  endif (NOT PNG_FOUND AND NOT DEFINED ENABLE_PNG)

endif (NOT ENABLE_PNG)
set(ENABLE_PNG "${ENABLE_PNG}" CACHE BOOL "Enable png build")

if (ENABLE_PNG)

  if (TARGET ZLIB_BLD)
    set(ZLIB_TARGET ZLIB_BLD)
  endif (TARGET ZLIB_BLD)

  ExternalProject_Add(PNG_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/png"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    $<$<BOOL:${ZLIB_TARGET}>:-DZ_PREFIX=ON>
    $<$<BOOL:${ZLIB_TARGET}>:-DZ_PREFIX_STR=${Z_PREFIX_STR}>
    $<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}>
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_LIBDIR=${LIB_DIR}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_INSTALL_RPATH=${CMAKE_INSTALL_PREFIX}/${LIB_DIR}
    -DPNG_LIB_NAME=${PNG_LIB_NAME}
    -DPNG_NO_DEBUG_POSTFIX=ON
    -DPNG_DEBUG_POSTFIX=""
    -DPNG_PREFIX=brl_
    -DPNG_STATIC=${BUILD_STATIC_LIBS}
    -DPNG_TESTS=OFF
    -DSKIP_INSTALL_EXECUTABLES=ON -DSKIP_INSTALL_FILES=ON
    -DSKIP_INSTALL_EXPORT=ON
    -DSKIP_INSTALL_EXPORT=ON
    -DZLIB_ROOT=$<$<BOOL:${ZLIB_TARGET}>:${CMAKE_INSTALL_PREFIX}>
    -Dld-version-script=OFF
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    )

  if (TARGET PATCH_BLD)
    ExternalProject_Add_StepDependencies(PNG_BLD configure PATCH_BLD-install)
  endif (TARGET PATCH_BLD)

  if (TARGET ZLIB_BLD)
    ExternalProject_Add_StepDependencies(PNG_BLD configure ZLIB_BLD-install)
  endif (TARGET ZLIB_BLD)

  ExternalProject_Add_StepTargets(PNG_BLD install)

  SetTargetFolder(PNG_BLD "Third Party Libraries")
  SetTargetFolder(png "Third Party Libraries")

endif (ENABLE_PNG)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

