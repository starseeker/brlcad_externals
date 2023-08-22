# Patchelf is only useful on some platforms - don't worry about trying
# to build it everywhere
if (NOT MSVC AND NOT APPLE)

  # Unless the user specifically requests it be built, only enable it if we
  # can't find a system version - patchelf is not bundled with BRL-CAD, so
  # a system version is fine if available.  We set up ENABLE_PATCHELF this
  # way so it will show as an option in the CMake GUI even without a
  # default value set.
  if (NOT ENABLE_PATCHELF)

    find_program(PATCHELF_EXECUTABLE patchelf)
    mark_as_advanced(PATCHELF_EXECUTABLE)

    if (NOT PATCHELF_EXECUTABLE AND NOT DEFINED ENABLE_PATCHELF)
      set(ENABLE_PATCHELF "ON" CACHE BOOL "Enable patchelf build")
    endif (NOT PATCHELF_EXECUTABLE AND NOT DEFINED ENABLE_PATCHELF)

  endif (NOT ENABLE_PATCHELF)
  set(ENABLE_PATCHELF "${ENABLE_PATCHELF}" CACHE BOOL "Enable patchelf build")

  if (ENABLE_PATCHELF)

    ExternalProject_Add(PATCHELF_BLD
      SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/patchelf"
      BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
      CMAKE_ARGS
      $<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}>
      -DBIN_DIR=${BIN_DIR}
      -DBUILD_STATIC_LIBS=${BUILD_STATIC_LIBS}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_INSTALL_PREFIX=${CMAKE_NOINSTALL_PREFIX}
      -DLIB_DIR=${LIB_DIR}
      LOG_CONFIGURE ${EXT_BUILD_QUIET}
      LOG_BUILD ${EXT_BUILD_QUIET}
      LOG_INSTALL ${EXT_BUILD_QUIET}
      LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
      )

    if (TARGET PATCH_BLD)
      ExternalProject_Add_StepDependencies(PATCHELF_BLD configure PATCH_BLD-install)
    endif (TARGET PATCH_BLD)

    ExternalProject_Add_StepTargets(PATCHELF_BLD install)

  endif (ENABLE_PATCHELF)

endif (NOT MSVC AND NOT APPLE)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

