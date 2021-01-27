cmake_minimum_required(VERSION 3.5)
project(HAWOpenCL LANGUAGES C)

enable_language(C)
include(CheckIncludeFiles)
include(FindOpenCL)

include(FindOpenGL)

# The version number.
set (${PROJECT_NAME}_VERSION_MAJOR 0)
set (${PROJECT_NAME}_VERSION_MINOR 1)

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
