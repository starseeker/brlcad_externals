# Unless we have ENABLE_ALL set, based the building of openimageio on
# the system detection results
if (ENABLE_ALL AND NOT DEFINED ENABLE_OPENIMAGEIO)
  set(ENABLE_OPENIMAGEIO ON)
endif (ENABLE_ALL AND NOT DEFINED ENABLE_OPENIMAGEIO)

if (NOT ENABLE_OPENIMAGEIO)

  find_package(OPENIMAGEIO)

  if (NOT OPENIMAGEIO_FOUND AND NOT DEFINED ENABLE_OPENIMAGEIO)
    set(ENABLE_OPENIMAGEIO "ON" CACHE BOOL "Enable openimageio build")
  endif (NOT OPENIMAGEIO_FOUND AND NOT DEFINED ENABLE_OPENIMAGEIO)

endif (NOT ENABLE_OPENIMAGEIO)
set(ENABLE_OPENIMAGEIO "${ENABLE_OPENIMAGEIO}" CACHE BOOL "Enable openimageio build")

if (ENABLE_OPENIMAGEIO)

  ExternalProject_Add(OPENIMAGEIO_BLD
    URL "${CMAKE_CURRENT_SOURCE_DIR}/openimageio"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    ${BUILD_TYPE_SPECIFIER}
    -DBUILD_DOCS=OFF
    -DOIIO_BUILD_TESTS=OFF
    -DOIIO_BUILD_TOOLS=OFF
    -DUSE_QT=OFF
    -DUSE_PYTHON=OFF
    -DUSE_RD3SDK=OFF
    -DBIN_DIR=${BIN_DIR}
    -DLIB_DIR=${LIB_DIR}
    -DBUILD_STATIC_LIBS=${BUILD_STATIC_LIBS}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BUNDLE_INSTALL_PREFIX}
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    STEP_TARGETS install
    )

  if (TARGET PATCH_BLD)
    ExternalProject_Add_StepDependencies(OPENIMAGEIO_BLD configure PATCH_BLD-install)
  endif (TARGET PATCH_BLD)

  if (TARGET TIFF_BLD)
    ExternalProject_Add_StepDependencies(OPENIMAGEIO_BLD configure TIFF_BLD-install)
  endif (TARGET TIFF_BLD)

  if (TARGET JPEG_BLD)
    ExternalProject_Add_StepDependencies(OPENIMAGEIO_BLD configure JPEG_BLD-install)
  endif (TARGET JPEG_BLD)

  IF (TARGET BOOST_BLD)
    EXTERNALPROJECT_ADD_STEPDEPENDENCIES(OPENIMAGEIO_BLD configure BOOST_BLD-install)
  endif (TARGET BOOST_BLD)

  if (TARGET FMT_BLD)
    ExternalProject_Add_StepDependencies(OPENIMAGEIO_BLD configure FMT_BLD-install)
  endif (TARGET FMT_BLD)

  if (TARGET OPENEXR_BLD)
    ExternalProject_Add_StepDependencies(OPENIMAGEIO_BLD configure OPENEXR_BLD-install)
  endif (TARGET OPENEXR_BLD)

endif (ENABLE_OPENIMAGEIO)

# Copy the license into position in CMAKE_BUNDLE_INSTALL_PREFIX
file(MAKE_DIRECTORY ${CMAKE_BUNDLE_INSTALL_PREFIX}/doc/legal/other)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/openimageio/LICENSE.md
  ${CMAKE_BUNDLE_INSTALL_PREFIX}/doc/legal/other/openimageio.txt
  COPYONLY
  )

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

