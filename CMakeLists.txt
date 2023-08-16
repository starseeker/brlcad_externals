#-----------------------------------------------------------------------
# Local Copies of External Libraries
#
# BRL-CAD depends on a variety of external libraries and tools -
# rather than fail if those requirements are not satisfied, we build
# local copies at need.
#
# There are three overall approaches to the handling of these
# dependencies:
#
# 1.  Auto - detect system libraries and use them if suitable,
#     otherwise build and use the local copy.  This is the default
#     approach.
#
# 2.  Bundled - regardless of system conditions, build and use all
#     bundled libraries.
#
# 3.  System - fail to build if the system libraries do not satisfy
#     requirements.  This is primarily useful for distributions that
#     want to ensure packages are using external libraries.
#
# In addition to the broad toplevel control, individual libraries can
# also be overridden - for example, if the toplevel setting is for
# Bundled libs, it is still possible to request a system library in
# individual cases.
#
# NOTE:  Logic in this file assumes the misc/tools directory has
# already been processed and relevant variables have been set.
#
#-----------------------------------------------------------------------

project(BDEPS)

cmake_minimum_required(VERSION 3.18)
if (POLICY CMP0114)
  cmake_policy(SET CMP0114 NEW)
endif (POLICY CMP0114)

# Location of CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# Define some variables
if(NOT DEFINED BIN_DIR)
  set(BIN_DIR bin)
endif(NOT DEFINED BIN_DIR)
if(NOT DEFINED INCLUDE_DIR)
  set(INCLUDE_DIR include)
endif(NOT DEFINED INCLUDE_DIR)
if(NOT DEFINED LIB_DIR)
  set(LIB_DIR lib)
endif(NOT DEFINED LIB_DIR)
if(NOT DEFINED DATA_DIR)
  set(DATA_DIR share)
endif(NOT DEFINED DATA_DIR)

# extinstall is the target for projects that will be incorporated into
# BRL-CAD bundles.  For tools like astyle and patchelf that are build
# tools only, we instead place them in extnoinstall.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/extinstall" CACHE PATH "Outputs for BRL-CAD bundling" FORCE)
set(CMAKE_NOINSTALL_PREFIX "${CMAKE_BINARY_DIR}/extnoinstall" CACHE PATH "Build tools NOT bundled with BRL-CAD" FORCE)

# If targets are built, we want to group them in build tools that support
# doing so.
function(SetTargetFolder targetname folder)
  if(TARGET ${targetname})
    set_target_properties(${targetname} PROPERTIES FOLDER "${folder}")
  endif(TARGET ${targetname})
endfunction(SetTargetFolder)

#---------------------------------------------------------------------
# By default, changes in src/other/ext files won't trigger a rebuild.  Setting
# this to 1 changes that, but at the expense of running the build step every
# time.  This may be fast if there is nothing to update in the project, but it
# will be slower than skipping the step completely.
if (NOT DEFINED EXT_BUILD_ALWAYS)
  set(EXT_BUILD_ALWAYS 0)
endif (NOT DEFINED EXT_BUILD_ALWAYS)

# The verbosity of the ExtProject stages is controlled by log options - they
# can either output to the console or to log files.  We establish a top level
# control for this - by default these outputs are logged, but if the verbose
# flag is set they will be printed to the console during build (useful in
# scenarios such as CI build runners, where it is advantageous to capture as
# much output as possible in the top level log.
if (EXT_BUILD_VERBOSE)
  set(EXT_BUILD_QUIET OFF)
else (EXT_BUILD_VERBOSE)
  set(EXT_BUILD_QUIET ON)
endif (EXT_BUILD_VERBOSE)

# Use this variable to key both CMake messages and tools like rpath_replace
if(NOT DEFINED EXTPROJ_VERBOSE)
  set(EXTPROJ_VERBOSE 0)
endif(NOT DEFINED EXTPROJ_VERBOSE)

# This project uses the superbuild pattern (see, for example,
# https://www.kitware.com/cmake-superbuilds-git-submodules) via CMake's
# ExternalProject_Add feature to manage 3rd party builds
include(ExternalProject)

# Custom patch utility to replace the build directory path with the install
# directory path in text files - make sure CMAKE_BINARY_DIR and
# CMAKE_INSTALL_PREFIX are finalized before generating this file!
#
# TODO - this probably belongs in the main BRL-CAD build as an install(CODE)
# step for files that need it...
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/buildpath_replace.cxx.in ${CMAKE_CURRENT_BINARY_DIR}/buildpath_replace.cxx)
add_executable(buildpath_replace ${CMAKE_CURRENT_BINARY_DIR}/buildpath_replace.cxx)
set_target_properties(buildpath_replace PROPERTIES FOLDER "Compilation Utilities")

# External build outputs can be verbose - capture them to files instead of
# outputting to console
#set(LOG_OPTS
#  LOG_DIR "${CMAKE_BINARY_DIR}/CMakeFiles/ExternalProject_logs"
#  LOG_CONFIGURE ON LOG_BUILD ON LOG_INSTALL ON
#  LOG_MERGED_STDOUTERR ON LOG_OUTPUT_ON_FAILURE ON
#  )

set(LOG_OPTS)

# In case we need to pass some sort of reasonable parallel
# flag to a build system, check our processor count
include(ProcessorCount)
ProcessorCount(N)
math(EXPR pcnt "${N} / 2")
if (NOT pcnt)
  set(pcnt 1)
endif (NOT pcnt)


# With MSVC, some of the non-CMake subbuilds are going to need the vcvars bat
# file to set up the environment.
if(MSVC)

  get_filename_component(COMPILER_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
  get_filename_component(COMPILER_ROOT "${COMPILER_DIR}" NAME)
  set(VCVARS_BAT "${COMPILER_DIR}/vcvars${COMPILER_ROOT}.bat")
  if(NOT EXISTS "${VCVARS_BAT}")
    # Try the VS2017 location (https://stackoverflow.com/q/43372235)
    set(COMPILER_ROOT "")
    foreach(DIRS RANGE 0 5)
      get_filename_component(COMPILER_DIR "${COMPILER_DIR}" DIRECTORY)
    endforeach(DIRS RANGE 0 5)
    set(COMPILER_DIR "${COMPILER_DIR}/Auxiliary/Build")
    if(CMAKE_CL_64)
      set(VCVARS_BAT "${COMPILER_DIR}/vcvars64.bat")
    else(CMAKE_CL_64)
      set(VCVARS_BAT "${COMPILER_DIR}/vcvars32.bat")
    endif(CMAKE_CL_64)
    if(NOT EXISTS "${VCVARS_BAT}")
      message(FATAL_ERROR "Could not find vcvars bat file in ${COMPILER_DIR}")
    endif(NOT EXISTS "${VCVARS_BAT}")
  endif(NOT EXISTS "${VCVARS_BAT}")

endif(MSVC)

###############################################################################
# Build tools are used during compilation, but are not bundled or distributed
# with BRL-CAD proper.
###############################################################################
include(${CMAKE_CURRENT_SOURCE_DIR}/astyle.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/patchelf.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/perplex.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/xmltools.cmake)


###############################################################################
# Build logic is broken out per-library, but the ordering is important.  Some
# libraries will depend on others listed here (for example, we want openNURBS
# to use our bundled zlib if it is enabled.) Developers adding, reordering or
# removing dependencies here need to make sure they are aware of impact they
# may be having on other external projects in other files.
###############################################################################

# zlib compression/decompression library
# https://zlib.net
#
# Note - our copy is modified from Vanilla upstream to support specifying a
# custom prefix - until a similar feature is available in upstream zlib, we
# need this to reliably avoid conflicts between bundled and system zlib.
include(${CMAKE_CURRENT_SOURCE_DIR}/zlib.cmake)

# libregex regular expression matching
include(${CMAKE_CURRENT_SOURCE_DIR}/regex.cmake)

# netpbm library - support for pnm,ppm,pbm, etc. image files
# http://netpbm.sourceforge.net/
#
# Note - we build a custom subset of this library for convenience, and (at the
# moment) mod it to remove a GPL string component, although there is some hope
# (2022) that the latter issue will be addressed upstream.  Arguably in this
# form our netpbm copy isn't really a good fit for ext, but it is kept here
# because a) there is an active upstream and b) we are unlikely to need to
# modify these sources to our needs from a functional perspective.
include(${CMAKE_CURRENT_SOURCE_DIR}/netpbm.cmake)

# libpng - Portable Network Graphics image file support
# http://www.libpng.org/pub/png/libpng.html
include(${CMAKE_CURRENT_SOURCE_DIR}/png.cmake)

# STEPcode - support for reading and writing STEP files
# https://github.com/stepcode/stepcode
#
# Note - We are heavily involved with the stepcode effort and in the past our
# stepcode copy has been extensively modified, but we are working to get our
# copy and a released upstream copy synced - in anticipation of that, stepcode
# lives in ext.
include(${CMAKE_CURRENT_SOURCE_DIR}/stepcode.cmake)

# SQLITE3 - embeddable database
include(${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.cmake)

# PROJ - generic coordinate transformation - https://proj.org
include(${CMAKE_CURRENT_SOURCE_DIR}/proj.cmake)

# GDAL -  translator library for raster and vector geospatial data formats
# https://gdal.org
#
# Note - the original inclusion of GDAL in BRL-CAD required a custom CMake
# build and source mods.  Upstream changes have largely eliminated the need for
# this, and work is pending to migrate our code to using vanilla upstream GDAL
# - hence, despite the mods, GDAL lives in ext in anticipation of that change.
include(${CMAKE_CURRENT_SOURCE_DIR}/gdal.cmake)

# Open Asset Import Library - library for supporting I/O for a number of
# Geometry file formats
# https://github.com/assimp/assimp
include(${CMAKE_CURRENT_SOURCE_DIR}/assetimport.cmake)

# OpenMesh Library - library for representing and manipulating polygonal meshes
# https://www.graphics.rwth-aachen.de/software/openmesh/
include(${CMAKE_CURRENT_SOURCE_DIR}/openmesh.cmake)

# TCL - scripting language.  For Tcl/Tk builds we want
# static lib building on so we get the stub libraries.
set(TCL_ENABLE_TK ON CACHE BOOL "enable tk")
include(${CMAKE_CURRENT_SOURCE_DIR}/tcl.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/tk.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/itcl.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/itk.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/iwidgets.cmake)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

