# For the patch tool, unless the user specifically requests it be built, only
# enable it if we can't find a system version - patch is not bundled with
# BRL-CAD, so a system version is fine if available.  We set up ENABLE_PATCH
# this way so it will show as an option in the CMake GUI even without a
# default value set.
set(PATCH_OPTIONS "--ignore-whitespace" CACHE STRING "Command line options to use when patching" FORCE)
if (NOT ENABLE_PATCH)

  find_program(PATCH_EXECUTABLE patch)
  mark_as_advanced(PATCH_EXECUTABLE)

  if (NOT PATCH_EXECUTABLE AND NOT DEFINED ENABLE_PATCH)
    set(ENABLE_PATCH "ON" CACHE BOOL "Enable patch build")
  endif (NOT PATCH_EXECUTABLE AND NOT DEFINED ENABLE_PATCH)

endif (NOT ENABLE_PATCH)
set(ENABLE_PATCH "${ENABLE_PATCH}" CACHE BOOL "Enable patch build")

if (ENABLE_PATCH)

  ExternalProject_Add(PATCH_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/patch"
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
    )


  ExternalProject_Add_StepTargets(PATCH_BLD install)

  SetTargetFolder(PATCH_BLD "Third Party Build Tools")

  set(PATCH_EXECUTABLE ${CMAKE_NOINSTALL_PREFIX}/${BIN_DIR}/sb_patch${CMAKE_EXECUTABLE_SUFFIX} CACHE STRING "Patch executable" FORCE)
  set(PATCH_OPTIONS "--ignore-whitespace;--newline-output;preserve" CACHE STRING "Command line options to use when patching" FORCE)

endif (ENABLE_PATCH)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

