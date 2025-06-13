cmake_minimum_required(VERSION 3.12)
project(Prism_Controllers VERSION 1.0.0 LANGUAGES CXX)

set(PRISM_CONTROLLERS_LIBRARY_NAME Prism_Controllers)

set(CONTROLLERS_SOURCES
    window_controller.cpp
)

set(CONTROLLERS_HEADERS
    public/controllers/window_controller.hpp
)

add_library(${PRISM_CONTROLLERS_LIBRARY_NAME} STATIC
    ${CONTROLLERS_SOURCES}
    ${CONTROLLERS_HEADERS}
)

add_dependencies(
    ${PRISM_CONTROLLERS_LIBRARY_NAME}
        glad    
        glfw
)

target_link_libraries(${PRISM_CONTROLLERS_LIBRARY_NAME}
    PUBLIC
        glad
        glfw
)

target_include_directories(${PRISM_CONTROLLERS_LIBRARY_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)