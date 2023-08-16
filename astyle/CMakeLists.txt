# Minimum required version of CMake
cmake_minimum_required(VERSION 3.18)

# Set CMake project name
project(ASTYLE)

if (NOT DEFINED BIN_DIR)
  set (BIN_DIR bin)
endif (NOT DEFINED BIN_DIR)

if (NOT DEFINED LIB_DIR)
  set (LIB_DIR lib)
endif (NOT DEFINED LIB_DIR)

set(ASTYLE_SRCS
  astyle_main.cpp
  ASBeautifier.cpp
  ASFormatter.cpp
  ASEnhancer.cpp
  ASLocalizer.cpp
  ASResource.cpp
  )

add_executable(astyle ${ASTYLE_SRCS})
set_target_properties(astyle PROPERTIES FOLDER "Compilation Utilities")

install(TARGETS astyle
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
