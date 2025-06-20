cmake_minimum_required(VERSION 3.14)
project(Prism_Loaders VERSION 1.0.0 LANGUAGES CXX)

set(PRISM_LOADERS_LIBRARY_NAME Prism_Loaders)

set(LOADERS_SOURCES
    glad_loader.cpp
    imgui_loader.cpp
    shader_loader.cpp
    mesh_loader.cpp
    window_loader.cpp
)

set(LOADERS_HEADERS
    public/loaders/glad_loader.hpp
    public/loaders/imgui_loader.hpp
    public/loaders/shader_loader.hpp
    public/loaders/mesh_loader.hpp
    public/loaders/window_loader.hpp
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
        assimp
        Prism_Resources
)

target_link_libraries(${PRISM_LOADERS_LIBRARY_NAME}
    PUBLIC
        glad
        glfw
        imgui
        assimp
        Prism_Resources
        Prism_Utils
)

target_include_directories(${PRISM_LOADERS_LIBRARY_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)