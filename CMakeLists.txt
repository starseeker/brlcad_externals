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
cmake_minimum_required(VERSION 3.19)

project(BDEPS)

if (POLICY CMP0114)
  cmake_policy(SET CMP0114 NEW)
endif (POLICY CMP0114)

# Require at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Location of CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# If we're not multiconfig and the user hasn't specified something,
# go with Release mode.  We're generally looking to bundle these
# as if they were system libraries, so that's usually going to
# mean release configuration.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT DEFINED CMAKE_BUILD_TYPE)

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

# extinstall is the target for projects that will be incorporated into BRL-CAD
# bundles.  For tools like astyle and plief that are build tools only, we
# instead place them in extnoinstall.

# Try to get the home directory
set(ENV_HOME "$ENV{HOME}")
if (NOT ENV_HOME)
  set(ENV_HOME "$ENV{USERPROFILE}")
endif (NOT ENV_HOME)

# If we're not being forced to a specific location
# and we know where home is, use it
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${ENV_HOME}")
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# If home didn't work, fall back on the build directory
if (NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
endif (NOT CMAKE_INSTALL_PREFIX)

# Having decided on the path, make sure subsequent configures stick with it
set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH "Calculated or specified install prefix" FORCE)

set(BDEPS_ROOT brlcad_ext)
set(CMAKE_BUNDLE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${BDEPS_ROOT}/extinstall" CACHE PATH "Outputs for BRL-CAD bundling" FORCE)
set(CMAKE_NOBUNDLE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${BDEPS_ROOT}/extnoinstall" CACHE PATH "Build tools NOT bundled with BRL-CAD" FORCE)
file(MAKE_DIRECTORY "${CMAKE_BUNDLE_INSTALL_PREFIX}")
file(MAKE_DIRECTORY "${CMAKE_NOBUNDLE_INSTALL_PREFIX}")

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

# Add a target that will run every time we build and touch a file in extnoinstall
# to allow interested CMake builds to be aware of potential changes, even if
# all that happens is to add files - otherwise, add-only operations won't trigger
# a CMake reconfigure even if the client build is watching the extinstall files.
file(MAKE_DIRECTORY ${CMAKE_NOBUNDLE_INSTALL_PREFIX})
add_custom_target(BuildStamp ALL COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_NOBUNDLE_INSTALL_PREFIX}/build-stamp)

###############################################################################
# Applying patch files cross platform is a challenging problem due to the
# shortage of portable tools and issues with LF vs CRLF files - see
# https://gitlab.kitware.com/cmake/cmake/-/issues/16854 for more background on
# this.  We need to address the patch build tool first thing, since any of the
# other ExternalProject_Add builds that follow may depend on it.
#
# The patch bundled here is from https://github.com/shannonbooth/patch
###############################################################################
add_subdirectory(patch)

###############################################################################
# Build tools are used during compilation, but are not bundled or distributed
# with BRL-CAD proper.  We group these tools here for convenience.  Unlike the
# build targets intended for distribution with BRL-CAD, these outputs are all
# targeted to extnoinstall.
###############################################################################
add_subdirectory(astyle)
add_subdirectory(lief)
add_subdirectory(perplex)
add_subdirectory(plief)
add_subdirectory(strclear)
add_subdirectory(xmltools)

###############################################################################
# Build logic is broken out per-library, but the ordering is important.  Some
# libraries will depend on others listed here (for example, we want openNURBS
# to use our bundled zlib if it is enabled.) Developers adding, reordering or
# removing dependencies here need to make sure they are aware of impact they
# may be having on other external projects in other files.
###############################################################################

# zlib compression/decompression library
# https://zlib.net
add_subdirectory(zlib)

# libregex regular expression matching
add_subdirectory(regex)

# netpbm library - support for pnm,ppm,pbm, etc. image files
# http://netpbm.sourceforge.net/
#
# Note - we build a custom subset of this library for convenience, and (at the
# moment) mod it to remove a GPL string component, although there is some hope
# (2022) that the latter issue will be addressed upstream.  Arguably in this
# form our netpbm copy isn't really a good fit for ext, but it is kept here
# because a) there is an active upstream and b) we are unlikely to need to
# modify these sources to our needs from a functional perspective.
add_subdirectory(netpbm)

# libpng - Portable Network Graphics image file support
# http://www.libpng.org/pub/png/libpng.html
add_subdirectory(png)

# STEPcode - support for reading and writing STEP files
# https://github.com/stepcode/stepcode
add_subdirectory(stepcode)

# SQLITE3 - embeddable database
# https://www.sqlite.org
add_subdirectory(sqlite3)

# PROJ - generic coordinate transformation
# https://proj.org
add_subdirectory(proj)

# GDAL -  translator library for raster and vector geospatial data formats
# https://gdal.org
add_subdirectory(gdal)

# Open Asset Import Library - library for supporting I/O for a number of
# Geometry file formats
# https://github.com/assimp/assimp
add_subdirectory(assetimport)

# OpenMesh Library - library for representing and manipulating polygonal meshes
# https://www.graphics.rwth-aachen.de/software/openmesh/
add_subdirectory(openmesh)

# OpenCV - Open Source Computer Vision Library: http://opencv.org
add_subdirectory(opencv)

# TCL - scripting language.  For Tcl/Tk builds we want
# static lib building on so we get the stub libraries.
# https://www.tcl.tk
set(TCL_ENABLE_TK ON CACHE BOOL "enable tk")
add_subdirectory(tcl)
add_subdirectory(tk)
add_subdirectory(itcl3)
add_subdirectory(itk3)
add_subdirectory(iwidgets)

# Qt - cross-platform user interface/application development toolkit
# https://download.qt.io/archive/qt
add_subdirectory(qt6)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

