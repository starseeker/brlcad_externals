# plief is a custom tool for rpath replacement using the LIEF
# framework:  https://github.com/lief-project/LIEF

ExternalProject_Add(PLIEF_BLD
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plief"
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

if (TARGET LIEF_BLD)
  ExternalProject_Add_StepDependencies(PLIEF_BLD configure LIEF_BLD-install)
endif (TARGET LIEF_BLD)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

