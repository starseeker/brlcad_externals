# Copyright Contributors to the Open Shading Language project.
# SPDX-License-Identifier: BSD-3-Clause
# https://github.com/AcademySoftwareFoundation/OpenShadingLanguage

# The 'testrender' executable
file(GLOB testrender_srcs *.cpp)

if (USE_OPTIX)
    set (testrender_cuda_srcs
        cuda/quad.cu
        cuda/optix_raytracer.cu
        cuda/sphere.cu
        cuda/wrapper.cu )

    # We only need to generate the PTX for rend_lib.cu if we are using OptiX 7.0+.
    # Some of OptiX 6 device functions defined in rend_lib.cu cannot be compiled
    # with NVCC, they must be compiled with clang using the LLVM_COMPILE_CUDA macro.
    if (OPTIX_VERSION VERSION_GREATER_EQUAL 7)
        list (APPEND testshade_cuda_srcs cuda/rend_lib.cu)
    endif ()

    # We need to make sure that the PTX files are regenerated whenever these
    # headers change.
    set (testrender_cuda_headers
         cuda/rend_lib.h )

    LLVM_COMPILE_CUDA (
        ${CMAKE_CURRENT_SOURCE_DIR}/cuda/rend_lib.cu
        ${testrender_cuda_headers}
        "rend_llvm_compiled_ops"
        rend_lib_bc_cpp
        "" )

    list (APPEND testrender_srcs ${rend_lib_bc_cpp})

    # Generate PTX for all of the CUDA files
    foreach (cudasrc ${testrender_cuda_srcs})
        NVCC_COMPILE ( ${cudasrc} "" ptx_generated "" )
        list (APPEND ptx_list ${ptx_generated})
    endforeach ()

    add_custom_target (testrender_ptx ALL
        DEPENDS ${ptx_list}
        SOURCES ${testrender_cuda_srcs} )

    # Install the PTX files in a fixed location so that they can be
    # loaded at run time
    install (FILES ${ptx_list}
             DESTINATION ${OSL_PTX_INSTALL_DIR})
endif()

if (CMAKE_COMPILER_IS_INTELCLANG)
    # To better match existing test results
    add_compile_options("-fp-model=precise")
    # Better performance is likely by not requiring a precise floating point 
    # model, although with slightly different numerical results.
endif ()

add_executable (testrender ${testrender_srcs})

target_link_libraries (testrender
    PRIVATE
        oslexec oslquery
        pugixml::pugixml)

osl_optix_target (testrender)

install ( TARGETS testrender RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
