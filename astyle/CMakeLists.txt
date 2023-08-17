# For the astyle tool, unless the user specifically requests it be built, only
# enable it if we can't find a system version - astyle is not bundled with
# BRL-CAD, so a system version is fine if available.  We set up ENABLE_ASTYLE
# this way so it will show as an option in the CMake GUI even without a
# default value set.
if (NOT ENABLE_ASTYLE)

  find_program(ASTYLE_EXECUTABLE astyle)
  mark_as_advanced(ASTYLE_EXECUTABLE)

  if (NOT ASTYLE_EXECUTABLE AND NOT DEFINED ENABLE_ASTYLE)
    set(ENABLE_ASTYLE "ON" CACHE BOOL "Enable astyle build")
  endif (NOT ASTYLE_EXECUTABLE AND NOT DEFINED ENABLE_ASTYLE)

endif (NOT ENABLE_ASTYLE)
set(ENABLE_ASTYLE "${ENABLE_ASTYLE}" CACHE BOOL "Enable astyle build")

if (ENABLE_ASTYLE)

  ExternalProject_Add(ASTYLE_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/astyle"
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

  if (TARGET PATCH_BLD)
    ExternalProject_Add_StepDependencies(ASTYLE_BLD configure PATCH_BLD-install)
  endif (TARGET PATCH_BLD)

  ExternalProject_Add_StepTargets(ASTYLE_BLD install)

  SetTargetFolder(ASTYLE_BLD "Third Party Build Tools")

endif (ENABLE_ASTYLE)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

