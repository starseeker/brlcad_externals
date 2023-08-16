# Unfortunately, there does not appear to be a reliable way to test for the
# presence of the IWidgets package on a system Tcl/Tk.  As far as I can tell
# the "package require Iwidgets" test (which is what is required to properly
# test for an available Iwidgets package) can ONLY be performed successfully on
# a system that supports creation of a graphics window. Window creation isn't
# typically available on continuous integration runners, which means the test
# will always fail there even when it shouldn't.

# Unless we have been specifically instructed not to, provide the bundled
# version of IWidgets.

if (NOT DEFINED ENABLE_IWIDGETS)
  set(ENABLE_IWIDGETS ON)
endif (NOT DEFINED ENABLE_IWIDGETS)

if (ENABLE_IWIDGETS)

  ExternalProject_Add(IWIDGETS_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/iwidgets"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DLIB_DIR=${LIB_DIR}
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    )

  if (TARGET TCL_BLD)
    ExternalProject_Add_StepDependencies(IWIDGETS_BLD configure TCL_BLD-install)
  endif (TARGET TCL_BLD)
  if (TARGET TK_BLD)
    ExternalProject_Add_StepDependencies(IWIDGETS_BLD configure TK_BLD-install)
  endif (TARGET TK_BLD)
  if (TARGET ITCL_BLD)
    ExternalProject_Add_StepDependencies(IWIDGETS_BLD configure ITCL_BLD-install)
  endif (TARGET ITCL_BLD)
  if (TARGET ITK_BLD)
    ExternalProject_Add_StepDependencies(IWIDGETS_BLD configure ITK_BLD-install)
  endif (TARGET ITK_BLD)

  SetTargetFolder(IWIDGETS_BLD "Third Party Libraries")

endif (ENABLE_IWIDGETS)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

