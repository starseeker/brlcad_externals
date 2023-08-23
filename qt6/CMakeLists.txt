# Unless we have ENABLE_ALL set, based the building of tcl on
# the system detection results
if (ENABLE_ALL AND NOT DEFINED ENABLE_QT)
  set(ENABLE_QT ON)
endif (ENABLE_ALL AND NOT DEFINED ENABLE_QT)

if (NOT ENABLE_QT)

  find_package(Qt6 COMPONENTS Core Widgets Gui Svg OpenGL OpenGLWidgets Network)

  if (NOT Qt6Widgets_FOUND AND NOT DEFINED ENABLE_QT)
    set(ENABLE_QT "ON" CACHE BOOL "Enable Qt6 build")
  endif (NOT Qt6Widgets_FOUND AND NOT DEFINED ENABLE_QT)

endif (NOT ENABLE_QT)

set(ENABLE_QT "${ENABLE_QT}" CACHE BOOL "Enable Qt6 build")

if (ENABLE_QT)

  if (NOT MSVC)

    ExternalProject_Add(Qt6_BLD
      SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/qtbase"
      BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
      CONFIGURE_COMMAND LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/${LIB_DIR} CPPFLAGS=-I${CMAKE_INSTALL_PREFIX}/${INCLUDE_DIR} LDFLAGS=-L${CMAKE_INSTALL_PREFIX}/${LIB_DIR}  ${CMAKE_CURRENT_SOURCE_DIR}/qtbase/configure -xcb --prefix=${CMAKE_INSTALL_PREFIX} --release
      BUILD_COMMAND ${CMAKE_COMMAND} --build . --parallel --config Release
      INSTALL_COMMAND ${CMAKE_COMMAND} --install .
      # Note - LOG_CONFIGURE doesn't seem to be compatible with complex CONFIGURE_COMMAND setups
      LOG_BUILD ${EXT_BUILD_QUIET}
      LOG_INSTALL ${EXT_BUILD_QUIET}
      LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
      )

  else (NOT MSVC)

    # For some reason running these commands directly seems to be problematic
    # with MSVC... wrapping them in CMake scripts, on the other hand, looks
    # like it's working...  not 100% sure why this is needed, unless there's
    # some bad interaction between the MSVC cmake and Visual Studio.
    set(QT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/qt6-build")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/qt_config.cmake "execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/qtbase/configure.bat --prefix=${CMAKE_INSTALL_PREFIX} --release WORKING_DIRECTORY ${QT_BUILD_DIR})")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/qt_build.cmake "execute_process(COMMAND cmake --build . --parallel --config Release WORKING_DIRECTORY ${QT_BUILD_DIR})")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/qt_install.cmake "execute_process(COMMAND cmake --install . WORKING_DIRECTORY ${QT_BUILD_DIR})")

    ExternalProject_Add(Qt6_BLD
      SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/qtbase"
      BINARY_DIR "${QT_BUILD_DIR}"
      BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
      CONFIGURE_COMMAND ${VCVARS_BAT} && cmake -P ${CMAKE_CURRENT_BINARY_DIR}/qt_config.cmake
      BUILD_COMMAND ${VCVARS_BAT} && cmake -P ${CMAKE_CURRENT_BINARY_DIR}/qt_build.cmake
      INSTALL_COMMAND ${VCVARS_BAT} && cmake -P ${CMAKE_CURRENT_BINARY_DIR}/qt_install.cmake
      # Note - LOG_CONFIGURE doesn't seem to be compatible with complex CONFIGURE_COMMAND setups
      LOG_BUILD ${EXT_BUILD_QUIET}
      LOG_INSTALL ${EXT_BUILD_QUIET}
      LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
      )

  endif (NOT MSVC)

  if (TARGET PATCH_BLD)
    ExternalProject_Add_StepDependencies(Qt6_BLD configure PATCH_BLD-install)
  endif (TARGET PATCH_BLD)

  ExternalProject_Add_StepTargets(Qt6_BLD install)

  if (TARGET ZLIB_BLD)
    set(ZLIB_TARGET ZLIB_BLD)
  endif (TARGET ZLIB_BLD)

  ExternalProject_Add(Qt6Svg_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/qtsvg"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    -DQt6_DIR=${CMAKE_INSTALL_PREFIX}
    -DQt6_ROOT=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    $<$<BOOL:${ZLIB_TARGET}>:-DZ_PREFIX=ON>
    $<$<BOOL:${ZLIB_TARGET}>:-DZ_PREFIX_STR=${Z_PREFIX_STR}>
    -DZLIB_ROOT=$<$<BOOL:${ZLIB_TARGET}>:${CMAKE_INSTALL_PREFIX}>
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    )

  if (TARGET PATCH_BLD)
    ExternalProject_Add_StepDependencies(Qt6_BLD configure PATCH_BLD-install)
  endif (TARGET PATCH_BLD)
  ExternalProject_Add_StepDependencies(Qt6Svg_BLD configure Qt6_BLD-install)


  ExternalProject_Add_StepTargets(Qt6Svg_BLD install)

endif (ENABLE_QT)

mark_as_advanced(Qt6_INCLUDE_DIRS)
mark_as_advanced(Qt6_LIBRARIES)
mark_as_advanced(Qt6_VERSION)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8


