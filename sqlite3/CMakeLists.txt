# Unless we have ENABLE_ALL set, based the building of png on
# the system detection results
if (ENABLE_ALL AND NOT DEFINED ENABLE_SQLite3)
  set(ENABLE_SQLite3 ON)
endif (ENABLE_ALL AND NOT DEFINED ENABLE_SQLite3)

if (NOT ENABLE_SQLite3)

  find_package(SQLite3)

  if (NOT SQLite3_FOUND AND NOT DEFINED ENABLE_SQLite3)
    set(ENABLE_SQLite3 "ON" CACHE BOOL "Enable sqlite3 build")
  endif (NOT SQLite3_FOUND AND NOT DEFINED ENABLE_SQLite3)

endif (NOT ENABLE_SQLite3)
set(ENABLE_SQLite3 "${ENABLE_SQLite3}" CACHE BOOL "Enable sqlite3 build")

if (ENABLE_SQLite3)

  ExternalProject_Add(SQLITE3_BLD
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sqlite3"
    BUILD_ALWAYS ${EXT_BUILD_ALWAYS} ${LOG_OPTS}
    CMAKE_ARGS
    $<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}>
    -DBIN_DIR=${BIN_DIR}
    -DLIB_DIR=${LIB_DIR}
    -DBUILD_STATIC_LIBS=${BUILD_STATIC_LIBS}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    LOG_CONFIGURE ${EXT_BUILD_QUIET}
    LOG_BUILD ${EXT_BUILD_QUIET}
    LOG_INSTALL ${EXT_BUILD_QUIET}
    LOG_OUTPUT_ON_FAILURE ${EXT_BUILD_QUIET}
    )

  ExternalProject_Add_StepTargets(SQLITE3_BLD install)

endif (ENABLE_SQLite3)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

