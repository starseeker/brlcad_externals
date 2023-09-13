# Unless we have ENABLE_ALL set, based the building of Boost on
# the system detection results
if (ENABLE_ALL AND NOT DEFINED ENABLE_BOOST)
  set(ENABLE_BOOST ON)
endif (ENABLE_ALL AND NOT DEFINED ENABLE_BOOST)

if (NOT ENABLE_BOOST)

  find_package(Boost)

  if (NOT Boost_FOUND AND NOT DEFINED ENABLE_BOOST)
    set(ENABLE_BOOST "ON" CACHE BOOL "Enable BOOST build")
  endif (NOT Boost_FOUND AND NOT DEFINED ENABLE_BOOST)

endif (NOT ENABLE_BOOST)

set(ENABLE_BOOST "${ENABLE_BOOST}" CACHE BOOL "Enable Boost build")

if (ENABLE_BOOST)
  if (isMultiConfig)
    set(CBT $<CONFIG>)
  else (isMultiConfig)
    set(CBT ${CMAKE_BUILD_TYPE})
  endif (isMultiConfig)

  set(BOOST_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/boost_srcs")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/boost_config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/boost_config.cmake @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/boost_build.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/boost_build.cmake @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/boost_install.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/boost_install.cmake @ONLY)

  ExternalProject_Add(BOOST_BLD
    URL "${CMAKE_CURRENT_SOURCE_DIR}/boost"
    SOURCE_DIR boost_srcs
    BUILD_IN_SOURCE ON
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CONFIGURE_COMMAND cmake -DCMAKE_BUILD_TYPE=${CBT} -P ${CMAKE_CURRENT_BINARY_DIR}/boost_config.cmake
    BUILD_COMMAND cmake  -DCMAKE_BUILD_TYPE=${CBT} -P ${CMAKE_CURRENT_BINARY_DIR}/boost_build.cmake
    INSTALL_COMMAND cmake -P ${CMAKE_CURRENT_BINARY_DIR}/boost_install.cmake
    # Note - LOG_CONFIGURE doesn't seem to be compatible with complex CONFIGURE_COMMAND setups
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    STEP_TARGETS install
    )

  if (TARGET PATCH_BLD)
    ExternalProject_Add_StepDependencies(BOOST_BLD configure PATCH_BLD-install)
  endif (TARGET PATCH_BLD)
endif (ENABLE_BOOST)

# Copy the license into position in CMAKE_BUNDLE_INSTALL_PREFIX
file(MAKE_DIRECTORY ${CMAKE_BUNDLE_INSTALL_PREFIX}/doc/legal/other)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/boost/LICENSE_1_0.txt
  ${CMAKE_BUNDLE_INSTALL_PREFIX}/doc/legal/other/boost.txt
  COPYONLY
  )

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8


