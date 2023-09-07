# plief is a custom tool for rpath replacement using the LIEF
# framework:  https://github.com/lief-project/LIEF

# We only need plief on some platforms - don't worry about trying to build it
# everywhere
if (NOT MSVC AND NOT APPLE)

  ExternalProject_Add(PLIEF_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plief"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    ${BUILD_TYPE_SPECIFIER}
    -DBIN_DIR=${BIN_DIR}
    -DLIB_DIR=${LIB_DIR}
    -DBUILD_STATIC_LIBS=${BUILD_STATIC_LIBS}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_NOBUNDLE_INSTALL_PREFIX}
    -DLIEF_DIR=${CMAKE_BUNDLE_INSTALL_PREFIX}/share/lief/cmake
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    STEP_TARGETS install
    )

  if (TARGET LIEF_BLD)
    ExternalProject_Add_StepDependencies(PLIEF_BLD configure LIEF_BLD-install)
  endif (TARGET LIEF_BLD)

endif (NOT MSVC AND NOT APPLE)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

