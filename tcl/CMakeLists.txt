# Unless we have ENABLE_ALL set, based the building of tcl on
# the system detection results
if (ENABLE_ALL AND NOT DEFINED ENABLE_TCL)
  set(ENABLE_TCL ON)
endif (ENABLE_ALL AND NOT DEFINED ENABLE_TCL)

set(TCL_VERSION "8.6")

if (NOT ENABLE_TCL)

  set(TCL_ENABLE_TK ON CACHE BOOL "enable tk")
  find_package(TCL)

  if (NOT TCL_FOUND AND NOT DEFINED ENABLE_TCL)
    set(ENABLE_TCL "ON" CACHE BOOL "Enable tcl build")
  endif (NOT TCL_FOUND AND NOT DEFINED ENABLE_TCL)

endif (NOT ENABLE_TCL)
set(ENABLE_TCL "${ENABLE_TCL}" CACHE BOOL "Enable tcl build")

if (ENABLE_TCL)

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
    configure_file(tcl_replace.cxx.in ${CMAKE_CURRENT_BINARY_DIR}/tcl_replace.cxx @ONLY)
    add_executable(tcl_replace ${CMAKE_CURRENT_BINARY_DIR}/tcl_replace.cxx)
    set_target_properties(tcl_replace PROPERTIES FOLDER "Compilation Utilities")
  endif (NOT TARGET tcl_replace)

  if (NOT MSVC)

    # Check for spaces in the source and build directories - those won't work
    # reliably with the Tcl autotools based build.
    if ("${CMAKE_CURRENT_SOURCE_DIR}" MATCHES ".* .*")
      message(FATAL_ERROR "Bundled Tcl enabled, but the path \"${CMAKE_CURRENT_SOURCE_DIR}\" contains spaces.  On this platform, Tcl uses autotools to build; paths with spaces are not supported.  To continue relocate your source directory to a path that does not use spaces.")
    endif ("${CMAKE_CURRENT_SOURCE_DIR}" MATCHES ".* .*")
    if ("${CMAKE_CURRENT_BINARY_DIR}" MATCHES ".* .*")
      message(FATAL_ERROR "Bundled Tcl enabled, but the path \"${CMAKE_CURRENT_BINARY_DIR}\" contains spaces.  On this platform, Tcl uses autotools to build; paths with spaces are not supported.  To continue you must select a build directory with a path that does not use spaces.")
    endif ("${CMAKE_CURRENT_BINARY_DIR}" MATCHES ".* .*")

    set(TCL_REWORK_FILES
      "${TCL_SRC_DIR}/unix/configure"
      "${TCL_SRC_DIR}/macosx/configure"
      "${TCL_SRC_DIR}/unix/tcl.m4"
      "${TCL_SRC_DIR}/unix/tclUnixInit.c"
      "${TCL_SRC_DIR}/generic/tclPkgConfig.c"
      )

    if (TARGET ZLIB_BLD)
      set(PCOMMAND "tcl_replace;${TCL_REWORK_FILES}")
    endif (TARGET ZLIB_BLD)

    ExternalProject_Add(TCL_BLD
      URL "${CMAKE_CURRENT_SOURCE_DIR}/tcl"
      BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
      PATCH_COMMAND ${PCOMMAND}
      CONFIGURE_COMMAND LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/${LIB_DIR} CPPFLAGS=-I${CMAKE_INSTALL_PREFIX}/${INCLUDE_DIR} LDFLAGS=-L${CMAKE_INSTALL_PREFIX}/${LIB_DIR} TCL_SHLIB_LD_EXTRAS=-L${CMAKE_INSTALL_PREFIX}/${LIB_DIR} ${TCL_SRC_DIR}/unix/configure --prefix=${CMAKE_INSTALL_PREFIX}
      BUILD_COMMAND make -j${pcnt}
      INSTALL_COMMAND make install
      DEPENDS ${ZLIB_TARGET} tcl_replace
      # Note - LOG_CONFIGURE doesn't seem to be compatible with complex CONFIGURE_COMMAND setups
      LOG_BUILD ${EXT_BUILD_QUIET}
      LOG_INSTALL ${EXT_BUILD_QUIET}
      LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
      )

    set(TCL_APPINIT tclAppInit.c)

  else (NOT MSVC)

    # TODO - how to pass Z_PREFIX through nmake so zlib.h has the correct prefix?  Is https://stackoverflow.com/a/11041834 what we need?  Also, do we need to patch makefile.vc to reference our zlib dll?
    ExternalProject_Add(TCL_BLD
      URL "${CMAKE_CURRENT_SOURCE_DIR}/tcl"
      BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
      CONFIGURE_COMMAND ""
      BINARY_DIR ${TCL_SRC_DIR}/win
      BUILD_COMMAND ${VCVARS_BAT} && nmake -f makefile.vc INSTALLDIR=${CMAKE_INSTALL_PREFIX} SUFX=
      INSTALL_COMMAND ${VCVARS_BAT} && nmake -f makefile.vc install INSTALLDIR=${CMAKE_INSTALL_PREFIX} SUFX=
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
  add_custom_target(tclsh_cpy ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/${BIN_DIR}/tclsh${TCL_VERSION}${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_INSTALL_PREFIX}/${BIN_DIR}/tclsh${CMAKE_EXECUTABLE_SUFFIX}
    DEPENDS TCL_BLD-install
    )

  # The library file permissions end up a bit strange - fix them
  find_program(CHMOD_EXECUTABLE chmod)
  mark_as_advanced(CHMOD_EXECUTABLE)
  if (CHMOD_EXECUTABLE)
    add_custom_target(tcl_permissionsfix ALL
      COMMAND ${CHMOD_EXECUTABLE} a-x,u+w ${CMAKE_INSTALL_PREFIX}/${LIB_DIR}/libtcl${TCL_VERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}
      COMMAND ${CHMOD_EXECUTABLE} a-x,u+w ${CMAKE_INSTALL_PREFIX}/${LIB_DIR}/libtclstub${TCL_VERSION}${CMAKE_STATIC_LIBRARY_SUFFIX}
      DEPENDS TCL_BLD-install
      )
  endif (CHMOD_EXECUTABLE)

  SetTargetFolder(TCL_BLD "Third Party Libraries")
  SetTargetFolder(tcl "Third Party Libraries")

endif (ENABLE_TCL)

mark_as_advanced(TCL_INCLUDE_DIRS)
mark_as_advanced(TCL_LIBRARIES)
mark_as_advanced(TCL_VERSION)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8


