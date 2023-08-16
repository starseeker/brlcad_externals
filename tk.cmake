# By the time we get here, we have run FindTCL and should know
# if we have TK.

if (NOT TK_LIBRARY OR TARGET TCL_BLD)
  set(TK_DO_BUILD 1)
else (NOT TK_LIBRARY OR TARGET TCL_BLD)
  set(TK_DO_BUILD 0)
endif (NOT TK_LIBRARY OR TARGET TCL_BLD)

if (TK_DO_BUILD)

  if (TARGET TCL_BLD)
    # If we're building against a compiled Tcl and not a system Tcl,
    # set some vars accordingly
    set(TCL_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/TCL_BLD-prefix/src/TCL_BLD")
    set(TCL_TARGET TCL_BLD)
  else (TARGET TCL_BLD)
    get_filename_component(TCLCONF_DIR "${TCL_LIBRARY}" DIRECTORY)
  endif (TARGET TCL_BLD)

  set(TK_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/TK_BLD-prefix/src/TK_BLD")

  if (NOT MSVC)

    # Check for spaces in the source and build directories - those won't work
    # reliably with the Tk autotools based build.
    if ("${CMAKE_CURRENT_SOURCE_DIR}" MATCHES ".* .*")
      message(FATAL_ERROR "Bundled Tk enabled, but the path \"${CMAKE_CURRENT_SOURCE_DIR}\" contains spaces.  On this platform, Tk uses autotools to build; paths with spaces are not supported.  To continue relocate your source directory to a path that does not use spaces.")
    endif ("${CMAKE_CURRENT_SOURCE_DIR}" MATCHES ".* .*")
    if ("${CMAKE_CURRENT_BINARY_DIR}" MATCHES ".* .*")
      message(FATAL_ERROR "Bundled Tk enabled, but the path \"${CMAKE_CURRENT_BINARY_DIR}\" contains spaces.  On this platform, Tk uses autotools to build; paths with spaces are not supported.  To continue you must select a build directory with a path that does not use spaces.")
    endif ("${CMAKE_CURRENT_BINARY_DIR}" MATCHES ".* .*")

    ExternalProject_Add(TK_BLD
      URL "${CMAKE_CURRENT_SOURCE_DIR}/tk"
      BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
      CONFIGURE_COMMAND LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/${LIB_DIR} CPPFLAGS=-I${CMAKE_INSTALL_PREFIX}/${INCLUDE_DIR} LDFLAGS=-L${CMAKE_INSTALL_PREFIX}/${LIB_DIR} TK_SHLIB_LD_EXTRAS=-L${CMAKE_INSTALL_PREFIX}/${LIB_DIR} ${TK_SRC_DIR}/unix/configure --prefix=${CMAKE_INSTALL_PREFIX} --with-tcl=$<IF:$<BOOL:${TCL_TARGET}>,${CMAKE_INSTALL_PREFIX}/${LIB_DIR},${TCLCONF_DIR}>
      BUILD_COMMAND make -j${pcnt}
      INSTALL_COMMAND make install
      DEPENDS ${TCL_TARGET}
      # Note - LOG_CONFIGURE doesn't seem to be compatible with complex CONFIGURE_COMMAND setups
      LOG_BUILD ${EXT_BUILD_QUIET}
      LOG_INSTALL ${EXT_BUILD_QUIET}
      LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
      )

    set(TK_APPINIT tkAppInit.c)

  else (NOT MSVC)

    set(TK_BASENAME tk${TCL_MAJOR_VERSION}${TCL_MINOR_VERSION})
    set(TK_STUBNAME tkstub${TCL_MAJOR_VERSION}${TCL_MINOR_VERSION})
    set(TK_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(TK_WISHNAME wish${TCL_MAJOR_VERSION}${TCL_MINOR_VERSION})

    ExternalProject_Add(TK_BLD
      URL "${CMAKE_CURRENT_SOURCE_DIR}/tk"
      BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
      CONFIGURE_COMMAND ""
      BINARY_DIR ${TK_SRC_DIR}/win
      BUILD_COMMAND ${VCVARS_BAT} && nmake -f makefile.vc INSTALLDIR=${CMAKE_INSTALL_PREFIX} TCLDIR=${TCL_SRC_DIR} SUFX=
      INSTALL_COMMAND ${VCVARS_BAT} && nmake -f makefile.vc install INSTALLDIR=${CMAKE_INSTALL_PREFIX} TCLDIR=${TCL_SRC_DIR} SUFX=
      DEPENDS ${TCL_TARGET}
      LOG_BUILD ${EXT_BUILD_QUIET}
      LOG_INSTALL ${EXT_BUILD_QUIET}
      LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
      )

    set(TK_APPINIT)

  endif (NOT MSVC)

  if (TARGET TCL_BLD)
    ExternalProject_Add_StepDependencies(TK_BLD configure TCL_BLD-install)
  endif (TARGET TCL_BLD)

  ExternalProject_Add_StepTargets(TK_BLD install)

  SetTargetFolder(TK_BLD "Third Party Libraries")
  SetTargetFolder(tk "Third Party Libraries")

endif (TK_DO_BUILD)

mark_as_advanced(TK_INCLUDE_DIRS)
mark_as_advanced(TK_LIBRARIES)
mark_as_advanced(TK_X11_GRAPHICS)
mark_as_advanced(HAVE_TK)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

