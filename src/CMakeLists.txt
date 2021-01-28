include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
#link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIR})
link_directories(${OpenCL_LIBRARY})

check_include_files("stdlib.h" HAVE_STDLIB_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("unistd.h" HAVE_UNISTD_H)

option (HAWOPENCL_WANT_OPENGL "If the OpenCL library should work with/on GPU using OpenGL")

# Wonderful: CMP0072 is warning ALL the time about the changed behaviour of find_package(OpenGL)
# At least, setting this option here (and not within the if) shows it only once.
set(OpenGL_GL_PREFERENCE GLVND CACHE STRING "Linux-only: if GLVND, use the vendor-neutral GL libraries (default). If LEGACY, use the legacy ones (might be necessary to have optirun/primusrun work)")
set_property(CACHE OpenGL_GL_PREFERENCE PROPERTY STRINGS GLVND LEGACY)
if(HAWOPENCL_WANT_OPENGL)
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIR})
    link_directories(${OPENGL_gl_LIBRARY})
endif()

add_library(HAWOpenCL STATIC
    opencl_get_devices.c
    opencl_init.c
    opencl_kernel_build.c
    opencl_kernel_info.c
    opencl_kernel_load.c
    opencl_kernel_print_info.c
    opencl_print_info.c
    opencl_printf_error.c
    opencl_profile_events.c)

install(TARGETS HAWOpenCL
    ARCHIVE DESTINATION lib
)
