cmake_minimum_required(VERSION 3.12)
project(Prism_Loaders VERSION 1.0.0 LANGUAGES CXX)

set(PRISM_LOADERS_LIBRARY_NAME Prism_Loaders)

set(LOADERS_SOURCES
    glad_loader.cpp
    imgui_loader.cpp
    shader_loader.cpp
)

set(LOADERS_HEADERS
    public/glad_loader.hpp
    public/imgui_loader.hpp
    public/shader_loader.hpp
)

add_library(${PRISM_LOADERS_LIBRARY_NAME} STATIC
    ${LOADERS_SOURCES}
    ${LOADERS_HEADERS}
)

add_dependencies(
    ${PRISM_LOADERS_LIBRARY_NAME}
        glad
        glfw
        imgui
)

target_link_libraries(${PRISM_LOADERS_LIBRARY_NAME}
    PUBLIC
        glad
        glfw
        imgui
)

target_include_directories(${PRISM_LOADERS_LIBRARY_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)