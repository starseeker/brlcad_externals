# TODO - We appear to have a problem with spaces in pathnames and the 8.6 autotools
# build. (https://core.tcl-lang.org/tcl/tktview/888c1a8c9d84c1f5da4a46352bbf531424fe7126)
# May have to switch back to the CMake system until that's fixed, if we want to
# keep the odd pathnames test running with bundled libs...

if (BRLCAD_ENABLE_TCL)
  set(TCL_VERSION "8.6" CACHE STRING "BRL-CAD uses Tcl 8.6" FORCE)
endif (BRLCAD_ENABLE_TCL)

set(tcl_DESCRIPTION "
Option for enabling and disabling compilation of the Tcl library
provided with BRL-CAD's source code.  Default is AUTO, responsive to
the toplevel BRLCAD_BUNDLED_LIBS option and testing first for a system
version if BRLCAD_BUNDLED_LIBS is also AUTO.
")

THIRD_PARTY(tcl TCL tcl
  tcl_DESCRIPTION
  ALIASES ENABLE_TCL
  RESET_VARS TCL_LIBRARY TCL_LIBRARIES TCL_STUB_LIBRARY TCL_TCLSH TCL_INCLUDE_PATH TCL_INCLUDE_DIRS
  )

if (BRLCAD_TCL_BUILD)

  set(TCL_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/TCL_BLD-prefix/src/TCL_BLD")

  # In addition to the usual target dependencies, we need to adjust for the
  # non-standard BRL-CAD zlib name, if we are using our bundled version.  Set a
  # variable here so the tcl_replace utility will know the right value.
  if (TARGET ZLIB_BLD)
    set(ZLIB_TARGET ZLIB_BLD)
    set(ZLIB_NAME z_brl)
    set(DEFLATE_NAME brl_deflateSetHeader)
  else (TARGET ZLIB_BLD)
    set(ZLIB_NAME z)
    set(DEFLATE_NAME deflateSetHeader)
  endif (TARGET ZLIB_BLD)

  # We need to set internal Tcl variables to the final install paths, not the intermediate install paths that
  # Tcl's own build will think are the final paths.  Rather than attempt build system trickery we simply
  # hard set the values in the source files by rewriting them.
  if (NOT TARGET tcl_replace)
    configure_file(${BDEPS_CMAKE_DIR}/tcl_replace.cxx.in ${CMAKE_CURRENT_BINARY_DIR}/tcl_replace.cxx @ONLY)
    add_executable(tcl_replace ${CMAKE_CURRENT_BINARY_DIR}/tcl_replace.cxx)
    set_target_properties(tcl_replace PROPERTIES FOLDER "Compilation Utilities")
  endif (NOT TARGET tcl_replace)

  #set(TCL_INSTDIR ${CMAKE_BINARY_INSTALL_ROOT}/tcl)
  set(TCL_INSTDIR ${CMAKE_BINARY_INSTALL_ROOT})

  if (NOT MSVC)

    # Bundled Tcl is a poison pill for the odd_pathnames distcheck test
    set(BRLCAD_DISABLE_ODD_PATHNAMES_TEST ON CACHE BOOL "Bundled disable by Tcl of distcheck-odd_pathnames")
    mark_as_advanced(BRLCAD_DISABLE_ODD_PATHNAMES_TEST)

    # Check for spaces in the source and build directories - those won't work
    # reliably with the Tcl autotools based build.
    if ("${CMAKE_CURRENT_SOURCE_DIR}" MATCHES ".* .*")
      message(FATAL_ERROR "Bundled Tcl enabled, but the path \"${CMAKE_CURRENT_SOURCE_DIR}\" contains spaces.  On this platform, Tcl uses autotools to build; paths with spaces are not supported.  To continue relocate your source directory to a path that does not use spaces.")
    endif ("${CMAKE_CURRENT_SOURCE_DIR}" MATCHES ".* .*")
    if ("${CMAKE_CURRENT_BINARY_DIR}" MATCHES ".* .*")
      message(FATAL_ERROR "Bundled Tcl enabled, but the path \"${CMAKE_CURRENT_BINARY_DIR}\" contains spaces.  On this platform, Tcl uses autotools to build; paths with spaces are not supported.  To continue you must select a build directory with a path that does not use spaces.")
    endif ("${CMAKE_CURRENT_BINARY_DIR}" MATCHES ".* .*")

    if (OPENBSD)
      set(TCL_BASENAME libtcl${TCL_MAJOR_VERSION}${TCL_MINOR_VERSION})
      set(TCL_STUBNAME libtclstub${TCL_MAJOR_VERSION}${TCL_MINOR_VERSION})
      set(TCL_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}.1.0)
    else (OPENBSD)
      set(TCL_BASENAME libtcl${TCL_MAJOR_VERSION}.${TCL_MINOR_VERSION})
      set(TCL_STUBNAME libtclstub${TCL_MAJOR_VERSION}.${TCL_MINOR_VERSION})
      set(TCL_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif (OPENBSD)
    set(TCL_EXECNAME tclsh${TCL_MAJOR_VERSION}.${TCL_MINOR_VERSION})

    set(TCL_PATCH_FILES "${TCL_SRC_DIR}/unix/configure" "${TCL_SRC_DIR}/macosx/configure" "${TCL_SRC_DIR}/unix/tcl.m4")
    set(TCL_REWORK_FILES ${TCL_PATCH_FILES} "${TCL_SRC_DIR}/unix/tclUnixInit.c" "${TCL_SRC_DIR}/generic/tclPkgConfig.c")

    ExternalProject_Add(TCL_BLD
      URL "${CMAKE_CURRENT_SOURCE_DIR}/tcl"
      BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
      PATCH_COMMAND rpath_replace ${TCL_PATCH_FILES}
      COMMAND tcl_replace ${TCL_REWORK_FILES}
      CONFIGURE_COMMAND LD_LIBRARY_PATH=${CMAKE_BINARY_ROOT}/${LIB_DIR} CPPFLAGS=-I${CMAKE_BINARY_ROOT}/${INCLUDE_DIR} LDFLAGS=-L${CMAKE_BINARY_ROOT}/${LIB_DIR} TCL_SHLIB_LD_EXTRAS=-L${CMAKE_BINARY_ROOT}/${LIB_DIR} ${TCL_SRC_DIR}/unix/configure --prefix=${TCL_INSTDIR}
      BUILD_COMMAND make -j${pcnt}
      INSTALL_COMMAND make install
      DEPENDS ${ZLIB_TARGET} tcl_replace rpath_replace
      # Note - LOG_CONFIGURE doesn't seem to be compatible with complex CONFIGURE_COMMAND setups
      LOG_BUILD ${EXT_BUILD_QUIET}
      LOG_INSTALL ${EXT_BUILD_QUIET}
      LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
      )

    set(TCL_APPINIT tclAppInit.c)

  else (NOT MSVC)

    set(TCL_BASENAME tcl${TCL_MAJOR_VERSION}${TCL_MINOR_VERSION})
    set(TCL_STUBNAME tclstub${TCL_MAJOR_VERSION}${TCL_MINOR_VERSION})
    set(TCL_EXECNAME tclsh${TCL_MAJOR_VERSION}${TCL_MINOR_VERSION})
    set(TCL_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})

    # TODO - how to pass Z_PREFIX through nmake so zlib.h has the correct prefix?  Is https://stackoverflow.com/a/11041834 what we need?  Also, do we need to patch makefile.vc to reference our zlib dll?
    ExternalProject_Add(TCL_BLD
      URL "${CMAKE_CURRENT_SOURCE_DIR}/tcl"
      BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
      CONFIGURE_COMMAND ""
      BINARY_DIR ${TCL_SRC_DIR}/win
      BUILD_COMMAND ${VCVARS_BAT} && nmake -f makefile.vc INSTALLDIR=${TCL_INSTDIR} SUFX=
      INSTALL_COMMAND ${VCVARS_BAT} && nmake -f makefile.vc install INSTALLDIR=${TCL_INSTDIR} SUFX=
      DEPENDS ${ZLIB_TARGET} tcl_replace
      LOG_BUILD ${EXT_BUILD_QUIET}
      LOG_INSTALL ${EXT_BUILD_QUIET}
      LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
      )
    set(TCL_APPINIT)

  endif (NOT MSVC)

  if (TARGET ZLIB_BLD)
    ExternalProject_Add_StepDependencies(TCL_BLD configure ZLIB_BLD-install)
  endif (TARGET ZLIB_BLD)

  ExternalProject_Add_StepTargets(TCL_BLD install)

  # Scripts expect a non-versioned tclsh program, but the Tcl build doesn't provide one,
  # we must provide it ourselves
  #install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tclsh_exe> \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${BIN_DIR}/tclsh${CMAKE_EXECUTABLE_SUFFIX}\")")

  SetTargetFolder(TCL_BLD "Third Party Libraries")
  SetTargetFolder(tcl "Third Party Libraries")

endif (BRLCAD_TCL_BUILD)

mark_as_advanced(TCL_INCLUDE_DIRS)
mark_as_advanced(TCL_LIBRARIES)
mark_as_advanced(TCL_VERSION)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8


