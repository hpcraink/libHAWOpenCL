include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIR})
link_directories(${OpenCL_LIBRARY})

enable_testing()

if(HAWOPENCL_WANT_OPENGL)
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIR})
    link_directories(${OPENGL_gl_LIBRARY})
endif()

add_executable (opencl_print_info opencl_print_info.c) 
target_link_libraries(opencl_print_info HAWOpenCL ${OpenCL_LIBRARIES} ${OPENGL_LIBRARIES})

add_executable (opencl_vector_add opencl_vector_add.c) 
target_link_libraries(opencl_vector_add HAWOpenCL ${OpenCL_LIBRARIES} ${OPENGL_LIBRARIES})


install(TARGETS opencl_print_info
        DESTINATION bin
        CONFIGURATIONS Release RelWithDebInfo Debug
)
