if (ENABLE_TCL AND NOT DEFINED ENABLE_ITCL)
  set(ENABLE_ITCL ON)
endif (ENABLE_TCL AND NOT DEFINED ENABLE_ITCL)

if (NOT ENABLE_ITCL)

  find_package(ITCL)

  if (NOT ITCL_FOUND AND NOT DEFINED ENABLE_ITCL)
    set(ENABLE_ITCL "ON" CACHE BOOL "Enable itcl build")
  endif (NOT ITCL_FOUND AND NOT DEFINED ENABLE_ITCL)

endif (NOT ENABLE_ITCL)
set(ENABLE_ITCL "${ENABLE_ITCL}" CACHE BOOL "Enable itcl build")

if (ENABLE_ITCL)

  # If we're building ITCL, it's path setup must take into account the
  # subdirectory in which we are storing the library.
  set(RPATH_SUFFIX itcl3.4)

  if (TARGET TCL_BLD)
    set(TCL_TARGET TCL_BLD)
  endif (TARGET TCL_BLD)

  ExternalProject_Add(ITCL_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/itcl3"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    $<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}>
    -DBIN_DIR=${BIN_DIR}
    -DLIB_DIR=${LIB_DIR}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_INSTALL_RPATH=${CMAKE_INSTALL_PREFIX}/${LIB_DIR}/${RPATH_SUFFIX}
    -DINCLUDE_DIR=${INCLUDE_DIR}
    -DSHARED_DIR=${LIB_DIR}
    -DTCL_ENABLE_TK=ON
    -DTCL_ROOT=$<$<BOOL:${TCL_TARGET}>:${CMAKE_INSTALL_PREFIX}>
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    )

  ExternalProject_Add_StepTargets(ITCL_BLD install)

  if (TARGET TCL_BLD)
    ExternalProject_Add_StepDependencies(ITCL_BLD configure TCL_BLD-install)
  endif (TARGET TCL_BLD)
  if (TARGET TK_BLD)
    ExternalProject_Add_StepDependencies(ITCL_BLD configure TK_BLD-install)
  endif (TARGET TK_BLD)

  SetTargetFolder(ITCL_BLD "Third Party Libraries")

endif (ENABLE_ITCL)

mark_as_advanced(ITCL_LIBRARY)
mark_as_advanced(ITCL_LIBRARIES)
mark_as_advanced(ITCL_VERSION)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

