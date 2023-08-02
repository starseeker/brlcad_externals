# Wrap the platform specific naming conventions we must manage to copy
# build output from 3rd party build systems.
function(set_lib_vars RVAR root vmaj vmin vpatch)

  # OpenBSD has its own naming conventions.  Set a platform variable based on
  # the OS name so we can test for it succinctly.
  if ("${CMAKE_SYSTEM}" MATCHES ".*OpenBSD.*")
    set(OPENBSD ON)
  endif ("${CMAKE_SYSTEM}" MATCHES ".*OpenBSD.*")

  unset(${RVAR}_BASENAME)
  unset(${RVAR}_STATICNAME)
  unset(${RVAR}_SUFFIX)
  unset(${RVAR}_SYMLINK1)
  unset(${RVAR}_SYMLINK2)

  if (NOT "${vpatch}" STREQUAL "")
    set(vpatch_suffix ".${vpatch}")
  else()
    set(vpatch_suffix "")
  endif()

  if (MSVC)
    set(${RVAR}_BASENAME ${root})
    set(${RVAR}_STATICNAME ${root}-static)
    set(${RVAR}_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(${RVAR}_SYMLINK_1 ${${RVAR}_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(${RVAR}_SYMLINK_2 ${${RVAR}_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}.${vmaj})
  elseif (APPLE)
    set(${RVAR}_BASENAME lib${root})
    set(${RVAR}_STATICNAME lib${root})
    set(${RVAR}_SUFFIX .${vmaj}.${vmin}${vpatch_suffix}${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(${RVAR}_SYMLINK_1 ${${RVAR}_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(${RVAR}_SYMLINK_2 ${${RVAR}_BASENAME}.${vmaj}${CMAKE_SHARED_LIBRARY_SUFFIX})
  elseif (OPENBSD)
    set(${RVAR}_BASENAME lib${root})
    set(${RVAR}_STATICNAME lib${root})
    set(${RVAR}_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}.${vmaj}.${vmin})
  else (MSVC)
    set(${RVAR}_BASENAME lib${root})
    set(${RVAR}_STATICNAME lib${root})
    set(${RVAR}_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}.${vmaj}.${vmin}${vpatch_suffix})
    set(${RVAR}_SYMLINK_1 ${${RVAR}_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(${RVAR}_SYMLINK_2 ${${RVAR}_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}.${vmaj})
  endif (MSVC)

  # Communicate the answers to the parent scope - these are the return values.
  set(${RVAR}_BASENAME   "${${RVAR}_BASENAME}"   PARENT_SCOPE)
  set(${RVAR}_STATICNAME "${${RVAR}_STATICNAME}" PARENT_SCOPE)
  set(${RVAR}_SUFFIX     "${${RVAR}_SUFFIX}"     PARENT_SCOPE)
  set(${RVAR}_SYMLINK_1   "${${RVAR}_SYMLINK_1}"   PARENT_SCOPE)
  set(${RVAR}_SYMLINK_2   "${${RVAR}_SYMLINK_2}"   PARENT_SCOPE)

endfunction(set_lib_vars RVAR root)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

