# Minimum required version of CMake
cmake_minimum_required(VERSION 3.18)

# Set CMake project name
project(XSLTPROC)

if (NOT DEFINED BIN_DIR)
  set (BIN_DIR bin)
endif (NOT DEFINED BIN_DIR)

if (NOT DEFINED LIB_DIR)
  set (LIB_DIR lib)
endif (NOT DEFINED LIB_DIR)

if (NOT DEFINED CMAKE_INSTALL_RPATH)
  set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${LIB_DIR})
endif (NOT DEFINED CMAKE_INSTALL_RPATH)

include(CheckIncludeFile)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(time.h HAVE_TIME_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(stdarg.h HAVE_STDARG_H)

if(WIN32)
  add_definitions(-DWIN32 -D_WINDOWS -D_MBCS -DW3 -D_REENTRANT -D_DEBUG)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
  add_definitions(-DNOLIBTOOL -DHAVE_WIN32_THREADS -DHAVE_COMPILER_TLS)
endif(WIN32)

add_subdirectory(libxml)
add_subdirectory(libxslt)
add_subdirectory(libexslt)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/libxml/include
  ${CMAKE_CURRENT_SOURCE_DIR}/libxslt/include
  ${CMAKE_CURRENT_SOURCE_DIR}/libxslt/include/libxslt
  ${CMAKE_CURRENT_SOURCE_DIR}/libexslt/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include/libxslt
  /usr/local/include
  )

add_executable(xsltproc xsltproc.c)
target_link_libraries(xsltproc xml xslt exslt)
install(TARGETS xsltproc
  RUNTIME DESTINATION ${BIN_DIR}
  LIBRARY DESTINATION ${LIB_DIR}
  ARCHIVE DESTINATION ${LIB_DIR}
  )

add_executable(xmllint xmllint.c)
target_link_libraries(xmllint xml xslt exslt)
install(TARGETS xmllint
  RUNTIME DESTINATION ${BIN_DIR}
  LIBRARY DESTINATION ${LIB_DIR}
  ARCHIVE DESTINATION ${LIB_DIR}
  )

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
