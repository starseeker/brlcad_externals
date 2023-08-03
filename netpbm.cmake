set(netpbm_DESCRIPTION "
Option for enabling and disabling compilation of the netpbm library
provided with BRL-CAD's source code.  Default is AUTO, responsive to
the toplevel BRLCAD_BUNDLED_LIBS option and testing first for a system
version if BRLCAD_BUNDLED_LIBS is also AUTO.
")
THIRD_PARTY(netpbm NETPBM netpbm
  netpbm_DESCRIPTION
  ALIASES ENABLE_NETPBM
  RESET_VARS NETPBM_LIBRARY NETPBM_INCLUDE_DIR
  )

if (BRLCAD_NETPBM_BUILD)

  if (MSVC)
    set(NETPBM_BASENAME netpbm)
    set(NETPBM_STATICNAME netpbm-static)
  else (MSVC)
    set(NETPBM_BASENAME libnetpbm)
    set(NETPBM_STATICNAME libnetpbm)
  endif (MSVC)

  #set(NETPBM_INSTDIR ${CMAKE_BINARY_INSTALL_ROOT}/netpbm)
  set(NETPBM_INSTDIR ${CMAKE_BINARY_INSTALL_ROOT})

  ExternalProject_Add(NETPBM_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/netpbm"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    $<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}>
    -DBIN_DIR=${BIN_DIR}
    -DBUILD_STATIC_LIBS=${BUILD_STATIC_LIBS}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${NETPBM_INSTDIR}
    -DCMAKE_INSTALL_RPATH=${CMAKE_BUILD_RPATH}
    -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=${CMAKE_INSTALL_RPATH_USE_LINK_PATH}
    -DCMAKE_SKIP_BUILD_RPATH=${CMAKE_SKIP_BUILD_RPATH}
    -DLIB_DIR=${LIB_DIR}
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    )

  SetTargetFolder(NETPBM_BLD "Third Party Libraries")
  SetTargetFolder(netpbm "Third Party Libraries")

endif (BRLCAD_NETPBM_BUILD)

mark_as_advanced(NETPBM_INCLUDE_DIRS)
mark_as_advanced(NETPBM_LIBRARIES)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

