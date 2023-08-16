# Unless we have ENABLE_ALL set, based the building of png on
# the system detection results
if (ENABLE_ALL AND NOT DEFINED ENABLE_OPENMESH)
  set(ENABLE_OPENMESH ON)
endif (ENABLE_ALL AND NOT DEFINED ENABLE_OPENMESH)

if (NOT ENABLE_OPENMESH)

  # We generally don't want the Mac framework libpng...
  set(CMAKE_FIND_FRAMEWORK LAST)

  find_package(OpenMesh)

  if (NOT OpenMesh_FOUND AND NOT DEFINED ENABLE_OPENMESH)
    set(ENABLE_OPENMESH "ON" CACHE BOOL "Enable openmesh build")
  endif (NOT OpenMesh_FOUND AND NOT DEFINED ENABLE_OPENMESH)

endif (NOT ENABLE_OPENMESH)
set(ENABLE_OPENMESH "${ENABLE_OPENMESH}" CACHE BOOL "Enable openmesh build")

if (ENABLE_OPENMESH)

  ExternalProject_Add(OPENMESH_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openmesh"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    $<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}>
    -DBIN_DIR=${BIN_DIR}
    -DLIB_DIR=${LIB_DIR}
    -DBUILD_STATIC_LIBS=${BUILD_STATIC_LIBS}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_INSTALL_RPATH=${CMAKE_INSTALL_PREFIX}/${LIB_DIR}
    -DBUILD_APPS=OFF
    -DOPENMESH_BUILD_SHARED=ON
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    )

  SetTargetFolder(OPENMESH_BLD "Third Party Libraries")
  SetTargetFolder(openmesh "Third Party Libraries")

  # OpenMesh generates windows dll's in the root directory unlike most other builds which
  # stuff them in bin/
  # move them over so our ext build logic can find them
  if(MSVC)
    set(OPENMESH_LIBS Core Tools)
    set(OPENMESH_BASENAME OpenMesh)
    set(OPENMESH_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    foreach(OMLIB ${OPENMESH_LIBS})
      add_custom_command(TARGET OPENMESH_BLD POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E rename
	${CMAKE_INSTALL_PREFIX}/${OPENMESH_BASENAME}${OMLIB}${OPENMESH_SUFFIX}
	${CMAKE_INSTALL_PREFIX}/${BIN_DIR}/${OPENMESH_BASENAME}${OMLIB}${OPENMESH_SUFFIX})
    endforeach(OMLIB ${OPENMESH_LIBS})
  endif(MSVC)

endif (ENABLE_OPENMESH)

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

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

