cmake_minimum_required(VERSION 3.5)
project(HAWOpenCL LANGUAGES C)

enable_language(C)
include(CheckCSourceCompiles)
include(CheckIncludeFiles)
include(FindOpenCL)

include(FindOpenGL)

# The version number.
set (${PROJECT_NAME}_VERSION_MAJOR 0)
set (${PROJECT_NAME}_VERSION_MINOR 1)

check_c_source_compiles(
    "
        static void f(char * ptr) __attribute__ ((nonnull(1)));
        int main(void) {return 0;}
    "
    HAVE___ATTRIBUTE__NONNULL
)

check_c_source_compiles(
    "
        static char * f(void) __attribute__ ((warn_unused_result));
        static char * f(void) { return (char*)0; }
        int main(void) { return (int)f() & 0xff;}
    "
    HAVE___ATTRIBUTE__WARN_UNUSED_RESULT
)

include_directories(${PROJECT_BINARY_DIR}/include)

add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(test)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/include/HAWOpenCL_config.h.in"
  "${PROJECT_BINARY_DIR}/include/HAWOpenCL_config.h"
)

install(FILES ${CMAKE_SOURCE_DIR}/include/HAWOpenCL.h
              ${CMAKE_CURRENT_BINARY_DIR}/include/HAWOpenCL_config.h
        DESTINATION include
)

install(FILES ${CMAKE_SOURCE_DIR}/share/FindLibHAWOpenCL.cmake
        DESTINATION share/cmake/Modules
)
