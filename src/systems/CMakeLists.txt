cmake_minimum_required(VERSION 3.12)
project(Prism_Systems VERSION 1.0.0 LANGUAGES CXX)

set(PRISM_SYSTEMS_LIBRARY_NAME Prism_Systems)

set(SYSTEMS_SOURCES
    screen_clearing_system.cpp
    mesh_drawing_system.cpp
    imgui_drawing_system.cpp
    present_system.cpp
    input_control_system.cpp
)

set(SYSTEMS_HEADERS
    public/systems/screen_clearing_system.hpp
    public/systems/mesh_drawing_system.hpp
    public/systems/imgui_drawing_system.hpp
    public/systems/present_system.hpp
    public/systems/input_control_system.hpp
)

add_library(${PRISM_SYSTEMS_LIBRARY_NAME} STATIC
    ${SYSTEMS_SOURCES}
    ${SYSTEMS_HEADERS}
)

add_dependencies(
    ${PRISM_SYSTEMS_LIBRARY_NAME}
        glad
        glfw
        Prism_Resources
        Prism_Loaders
)

target_link_libraries(${PRISM_SYSTEMS_LIBRARY_NAME}
    PUBLIC
        glad
        glfw
        Prism_Resources
        Prism_Loaders
)

target_include_directories(${PRISM_SYSTEMS_LIBRARY_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)