cmake_minimum_required(VERSION 3.14)
project(Prism_Managers VERSION 1.0.0 LANGUAGES CXX)

set(PRISM_MANAGERS_LIBRARY_NAME Prism_Managers)

set(MANAGERS_HEADERS
    public/managers/scene_draw_systems_manager.hpp
    public/managers/scene_update_systems_manager.hpp
)

set(MANAGERS_SOURCES
    scene_draw_systems_manager.cpp
    scene_update_systems_manager.cpp
)

add_library(${PRISM_MANAGERS_LIBRARY_NAME} STATIC
    ${MANAGERS_HEADERS}
    ${MANAGERS_SOURCES}
)

add_dependencies(
    ${PRISM_MANAGERS_LIBRARY_NAME}
        Prism_Systems
        Prism_Resources
)

target_link_libraries(${PRISM_MANAGERS_LIBRARY_NAME}
    PUBLIC
        Prism_Systems
        Prism_Resources
        Prism_UI
)

target_include_directories(${PRISM_MANAGERS_LIBRARY_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)