cmake_minimum_required(VERSION 3.12)
project(Prism_Resources VERSION 1.0.0 LANGUAGES CXX)

set(PRISM_RESOURCES_LIBRARY_NAME Prism_Resources)

set(RESOURCES_SOURCES
    shader_resource.cpp
    imgui_resource.cpp
    mesh_resource.cpp
    scene.cpp
)

set(RESOURCES_HEADERS
    public/resources/resource.hpp
    public/resources/shader_resource.hpp
    public/resources/imgui_resource.hpp
    public/resources/mesh_resource.hpp
    public/resources/scene.hpp
    public/resources/context_resources.hpp
)

add_library(${PRISM_RESOURCES_LIBRARY_NAME} STATIC
    ${RESOURCES_SOURCES}
    ${RESOURCES_HEADERS}
)

add_dependencies(
    ${PRISM_RESOURCES_LIBRARY_NAME}
        glad
        glfw
        glm
        imgui
        EnTT
        Prism_Components
)

target_link_libraries(${PRISM_RESOURCES_LIBRARY_NAME}
    PUBLIC
        glad
        glfw
        glm
        imgui
        EnTT
        Prism_Components
)

target_include_directories(${PRISM_RESOURCES_LIBRARY_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)