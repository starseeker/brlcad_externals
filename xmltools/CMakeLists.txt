# xmltools provides the xsltproc and xmllint tools for Docbook processing
if (ENABLE_ALL AND NOT DEFINED ENABLE_XMLTOOLS)
  set(ENABLE_XMLTOOLS ON)
endif (ENABLE_ALL AND NOT DEFINED ENABLE_XMLTOOLS)

if (NOT ENABLE_XMLTOOLS)

  find_program(XSLTPROC_EXECUTABLE xsltproc)
  mark_as_advanced(XSLTPROC_EXECUTABLE)

  find_program(XMLLINT_EXECUTABLE xmllint)
  mark_as_advanced(XMLLINT_EXECUTABLE)

  if (NOT XSLTPROC_EXECUTABLE OR NOT XMLLINT_EXECUTABLE)
    if (NOT DEFINED ENABLE_XMLTOOLS)
      set(ENABLE_XMLTOOLS "ON" CACHE BOOL "Enable perplex build")
    endif (NOT DEFINED ENABLE_XMLTOOLS)
  endif (NOT XSLTPROC_EXECUTABLE OR NOT XMLLINT_EXECUTABLE)

endif (NOT ENABLE_XMLTOOLS)
set(ENABLE_XMLTOOLS "${ENABLE_XMLTOOLS}" CACHE BOOL "Enable perplex build")

if (ENABLE_XMLTOOLS)

  set(XMLTOOLS_INSTDIR ${CMAKE_BINARY_INSTALL_ROOT}/../extnoinstall)

  ExternalProject_Add(XMLTOOLS_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xmltools"
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

  if (TARGET PATCH_BLD)
    ExternalProject_Add_StepDependencies(XMLTOOLS_BLD configure PATCH_BLD-install)
  endif (TARGET PATCH_BLD)

endif (ENABLE_XMLTOOLS)

# Copy the license into position in CMAKE_INSTALL_PREFIX
file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/doc/legal/other)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/xmltools/libxslt/Copyright
  ${CMAKE_INSTALL_PREFIX}/doc/legal/other/xslt.txt
  COPYONLY
  )
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/xmltools/libexslt/Copyright
  ${CMAKE_INSTALL_PREFIX}/doc/legal/other/exslt.txt
  COPYONLY
  )
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/xmltools/libxml/Copyright
  ${CMAKE_INSTALL_PREFIX}/doc/legal/other/xml.txt
  COPYONLY
  )

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

