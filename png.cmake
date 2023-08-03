# NOTE: we need to have libpng's internal call to find_package looking for zlib
# locate our local copy if we have one.  Defining the ZLIB_ROOT prefix for
# find_package and setting the library file to our custom library name is
# intended to do this (requires CMake 3.12).

set(png_DESCRIPTION "
Option for enabling and disabling compilation of the Portable Network
Graphics library provided with BRL-CAD's source distribution.  Default
is AUTO, responsive to the toplevel BRLCAD_BUNDLED_LIBS option and
testing first for a system version if BRLCAD_BUNDLED_LIBS is also
AUTO.
")

# We generally don't want the Mac framework libpng...
set(CMAKE_FIND_FRAMEWORK LAST)

THIRD_PARTY(png PNG png
  png_DESCRIPTION
  ALIASES ENABLE_PNG
  RESET_VARS PNG_LIBRARY_DEBUG PNG_LIBRARY_RELEASE
  )

if (BRLCAD_PNG_BUILD)

  #set(PNG_INSTDIR ${CMAKE_BINARY_INSTALL_ROOT}/png)
  set(PNG_INSTDIR ${CMAKE_BINARY_INSTALL_ROOT})

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
    -DCMAKE_INSTALL_PREFIX=${PNG_INSTDIR}
    -DCMAKE_INSTALL_RPATH=${CMAKE_BUILD_RPATH}
    -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=${CMAKE_INSTALL_RPATH_USE_LINK_PATH}
    -DCMAKE_SKIP_BUILD_RPATH=${CMAKE_SKIP_BUILD_RPATH}
    -DPNG_LIB_NAME=${PNG_LIB_NAME}
    -DPNG_NO_DEBUG_POSTFIX=ON
    -DPNG_DEBUG_POSTFIX=""
    -DPNG_PREFIX=brl_
    -DPNG_STATIC=${BUILD_STATIC_LIBS}
    -DPNG_TESTS=OFF
    -DSKIP_INSTALL_EXECUTABLES=ON -DSKIP_INSTALL_FILES=ON
    -DSKIP_INSTALL_EXPORT=ON
    -DSKIP_INSTALL_EXPORT=ON
    -DZLIB_ROOT=$<$<BOOL:${ZLIB_TARGET}>:${CMAKE_BINARY_INSTALL_ROOT}>
    -Dld-version-script=OFF
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    )

  if (TARGET ZLIB_BLD)
    ExternalProject_Add_StepDependencies(PNG_BLD configure ZLIB_BLD-install)
  endif (TARGET ZLIB_BLD)

  ExternalProject_Add_StepTargets(PNG_BLD install)

  SetTargetFolder(PNG_BLD "Third Party Libraries")
  SetTargetFolder(png "Third Party Libraries")

  DISTCLEAN("${CMAKE_CURRENT_BINARY_DIR}/PNG_BLD-prefix")

endif (BRLCAD_PNG_BUILD)

mark_as_advanced(PNG_PNG_INCLUDE_DIR)
mark_as_advanced(PNG_INCLUDE_DIRS)
mark_as_advanced(PNG_LIBRARIES)
mark_as_advanced(PNG_LIBRARY_DEBUG)
mark_as_advanced(PNG_LIBRARY_RELEASE)

include("${CMAKE_CURRENT_SOURCE_DIR}/png.dist")

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

