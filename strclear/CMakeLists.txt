# For the strclear tool, unless the user specifically requests it be built, only
# enable it if we can't find a system version - strclear is not bundled with
# BRL-CAD, so a system version is fine if available.  We set up ENABLE_STRCLEAR
# this way so it will show as an option in the CMake GUI even without a
# default value set.
if (NOT ENABLE_STRCLEAR)

  find_program(STRCLEAR_EXECUTABLE strclear)
  mark_as_advanced(STRCLEAR_EXECUTABLE)

  if (NOT STRCLEAR_EXECUTABLE AND NOT DEFINED ENABLE_STRCLEAR)
    set(ENABLE_STRCLEAR "ON" CACHE BOOL "Enable strclear build")
  endif (NOT STRCLEAR_EXECUTABLE AND NOT DEFINED ENABLE_STRCLEAR)

endif (NOT ENABLE_STRCLEAR)
set(ENABLE_STRCLEAR "${ENABLE_STRCLEAR}" CACHE BOOL "Enable strclear build")

if (ENABLE_STRCLEAR)

  ExternalProject_Add(STRCLEAR_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/strclear"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    $<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}>
    -DBIN_DIR=${BIN_DIR}
    -DLIB_DIR=${LIB_DIR}
    -DBUILD_STATIC_LIBS=${BUILD_STATIC_LIBS}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_NOINSTALL_PREFIX}
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    STEP_TARGETS install
    )

endif (ENABLE_STRCLEAR)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

