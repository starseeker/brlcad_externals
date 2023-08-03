# Unfortunately, there does not appear to be a reliable way to test for the
# presence of the IWidgets package on a system Tcl/Tk.  As far as I can tell
# the "package require Iwidgets" test (which is what is required to properly
# test for an available Iwidgets package) can ONLY be performed successfully on
# a system that supports creation of a graphics window. Window creation isn't
# typically available on continuous integration runners, which means the test
# will always fail there even when it shouldn't.

# Unless we have been specifically instructed to use a system version, provide
# the bundled version.

if (BRLCAD_ENABLE_TK)

  # Do what we can to make a sane decision on whether to build Itk
  set(DO_IWIDGETS_BUILD 1)
  if ("${BRLCAD_IWIDGETS}" STREQUAL "SYSTEM")
    set(DO_IWIDGETS_BUILD 0)
  endif ("${BRLCAD_IWIDGETS}" STREQUAL "SYSTEM")

  if (DO_IWIDGETS_BUILD)

    set(BRLCAD_IWIDGETS_BUILD "ON" CACHE STRING "Enable Iwidgets build" FORCE)

    set(IWIDGETS_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/IWIDGETS_BLD-prefix/src/IWIDGETS_BLD")

    set(IWIDGETS_MAJOR_VERSION 4)
    set(IWIDGETS_MINOR_VERSION 1)
    set(IWIDGETS_PATCH_VERSION 1)
    set(IWIDGETS_VERSION ${IWIDGETS_MAJOR_VERSION}.${IWIDGETS_MINOR_VERSION}.${IWIDGETS_PATCH_VERSION} CACHE STRING "IWidgets version")

    # If we have build targets, set the variables accordingly.  Otherwise,
    # we need to find the *Config.sh script locations.
    set(IWIDGETS_DEPS)
    if (TARGET TCL_BLD)
      list(APPEND IWIDGETS_DEPS TCL_BLD)
    endif (TARGET TCL_BLD)
    if (TARGET ITCL_BLD)
      list(APPEND IWIDGETS_DEPS ITCL_BLD)
    endif (TARGET ITCL_BLD)
    if (TARGET TK_BLD)
      list(APPEND IWIDGETS_DEPS TK_BLD)
    endif (TARGET TK_BLD)
    if (TARGET itk_stage)
      list(APPEND IWIDGETS_DEPS itk_stage)
    endif (TARGET itk_stage)

    # The Iwidgets build doesn't seem to work with Itk the same way it does with the other
    # dependencies - just point it to our local source copy
    set(ITK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/itk3")

    #set(IWIDGETS_INSTDIR "${CMAKE_BINARY_INSTALL_ROOT}/iwidgets")
    set(IWIDGETS_INSTDIR "${CMAKE_BINARY_INSTALL_ROOT}")

    ExternalProject_Add(IWIDGETS_BLD
      SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/iwidgets"
      BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
      CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=${IWIDGETS_INSTDIR}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DLIB_DIR=${LIB_DIR}
      DEPENDS ${IWIDGETS_DEPS}
      LOG_CONFIGURE ${EXT_BUILD_QUIET}
      LOG_BUILD ${EXT_BUILD_QUIET}
      LOG_INSTALL ${EXT_BUILD_QUIET}
      LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
      )

    SetTargetFolder(IWIDGETS_BLD "Third Party Libraries")

  endif (DO_IWIDGETS_BUILD)

else (BRLCAD_ENABLE_TK)

    # If we don't have a Tk enabled build, there's never any point in building
    # Iwidgets - note it is off for the configure summary
    set(BRLCAD_IWIDGETS_BUILD "OFF" CACHE STRING "Disable Iwidgets build" FORCE)

endif (BRLCAD_ENABLE_TK)

mark_as_advanced(IWIDGETS_VERSION)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

