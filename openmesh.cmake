set(openmesh_DESCRIPTION "
Option for enabling and disabling compilation of the OpenMesh 
Library provided with BRL-CAD's source code.  Default
is AUTO, responsive to the toplevel BRLCAD_BUNDLED_LIBS option and
testing first for a system version if BRLCAD_BUNDLED_LIBS is also
AUTO.
")
THIRD_PARTY(openmesh OPENMESH openmesh
  openmesh_DESCRIPTION
  FIND_NAME OpenMesh
  ALIASES ENABLE_OPENMESH
  RESET_VARS OPENMESH_LIBRARY OPENMESH_LIBRARIES OPENMESH_INCLUDE_DIR OPENMESH_INCLUDE_DIRS 
  )

if (BRLCAD_OPENMESH_BUILD)
  set(OM_MAJOR_VERSION 9)
  set(OM_MINOR_VERSION 0)
  set(OM_PATCH_VERSION 0)

  if (MSVC)
    set(OPENMESH_BASENAME OpenMesh)
    set(OPENMESH_STATICNAME OpenMesh-static)
    set(OPENMESH_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
  elseif (APPLE)
    set(OPENMESH_BASENAME libOpenMesh)
    set(OPENMESH_STATICNAME libOpenMesh)
    set(OPENMESH_SUFFIX .${OM_MAJOR_VERSION}.${OM_MINOR_VERSION}${CMAKE_SHARED_LIBRARY_SUFFIX})
  elseif (OPENBSD)
    set(OPENMESH_BASENAME libOpenMeshCore)
    set(OPENMESH_STATICNAME libOpenMeshCore)
    set(OPENMESH_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}.1.0)
  else (MSVC)
    set(OPENMESH_BASENAME libOpenMesh)
    set(OPENMESH_STATICNAME libOpenMesh)
    set(OPENMESH_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}.${OM_MAJOR_VERSION}.${OM_MINOR_VERSION})
  endif (MSVC)

  #set(OPENMESH_INSTDIR ${CMAKE_BINARY_INSTALL_ROOT}/openmesh)
  set(OPENMESH_INSTDIR ${CMAKE_BINARY_INSTALL_ROOT})

  ExternalProject_Add(OPENMESH_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openmesh"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    $<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}>
    -DBIN_DIR=${BIN_DIR}
    -DBUILD_STATIC_LIBS=${BUILD_STATIC_LIBS}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${OPENMESH_INSTDIR}
    -DCMAKE_INSTALL_RPATH=${CMAKE_BUILD_RPATH}
    -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=${CMAKE_INSTALL_RPATH_USE_LINK_PATH}
    -DCMAKE_SKIP_BUILD_RPATH=${CMAKE_SKIP_BUILD_RPATH}
    -DLIB_DIR=${LIB_DIR}
    -DBUILD_APPS=OFF
    -DOPENMESH_BUILD_SHARED=ON
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    )

  SetTargetFolder(OPENMESH_BLD "Third Party Libraries")
  SetTargetFolder(openmesh "Third Party Libraries")

  DISTCLEAN("${CMAKE_CURRENT_BINARY_DIR}/OPENMESH_BLD-prefix")

  # OpenMesh generates windows dll's in the root directory unlike most other builds which
  # stuff them in bin/
  # copy them over so our ext build logic can find them
  if(MSVC)
    foreach(OMLIB ${OPENMESH_LIBS})
    add_custom_command(TARGET OPENMESH_BLD POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${OPENMESH_INSTDIR}/${OPENMESH_BASENAME}${OMLIB}${OPENMESH_SUFFIX}
      ${OPENMESH_INSTDIR}/bin/${OPENMESH_BASENAME}${OMLIB}${OPENMESH_SUFFIX})
    endforeach(OMLIB ${OPENMESH_LIBS})
  endif(MSVC)

endif (BRLCAD_OPENMESH_BUILD)

mark_as_advanced(OPENMESH_INCLUDE_DIRS)
mark_as_advanced(OPENMESH_LIBRARIES)
mark_as_advanced(OPENMESH_CORE_GEOMETRY_DIR)
mark_as_advanced(OPENMESH_CORE_IO_DIR)
mark_as_advanced(OPENMESH_CORE_MESH_DIR)
mark_as_advanced(OPENMESH_CORE_SYSTEM_DIR)
mark_as_advanced(OPENMESH_CORE_UTILS_DIR)
mark_as_advanced(OPENMESH_TOOLS_DECIMATER_DIR)
mark_as_advanced(OPENMESH_TOOLS_DUALIZER_DIR)
mark_as_advanced(OPENMESH_TOOLS_KERNERL_OSG_DIR)
mark_as_advanced(OPENMESH_TOOLS_SMOOTHER_DIR)
mark_as_advanced(OPENMESH_TOOLS_SUBDIVIDER_DIR)
mark_as_advanced(OPENMESH_TOOLS_UTILS_DIR)
mark_as_advanced(OPENMESH_TOOLS_VDPM_DIR)

include("${CMAKE_CURRENT_SOURCE_DIR}/openmesh.dist")

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

