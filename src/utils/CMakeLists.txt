cmake_minimum_required(VERSION 3.14)
project(Prism_Utils VERSION 1.0.0 LANGUAGES CXX)

set(PRISM_UTILS_LIBRARY_NAME Prism_Utils)

set(UTILS_SOURCES

)

set(UTILS_HEADERS
    public/utils/opengl_debug.hpp
)

add_library(${PRISM_UTILS_LIBRARY_NAME} INTERFACE ${UTILS_HEADERRS})

add_dependencies(
    ${PRISM_UTILS_LIBRARY_NAME}
    glad
    glfw
)

target_link_libraries(
    ${PRISM_UTILS_LIBRARY_NAME} 
    INTERFACE
        glad glfw
)

target_include_directories(${PRISM_UTILS_LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/public
)