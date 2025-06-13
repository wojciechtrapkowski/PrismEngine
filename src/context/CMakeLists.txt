cmake_minimum_required(VERSION 3.12)
project(Prism_Context VERSION 1.0.0 LANGUAGES CXX)

set(PRISM_CONTEXT_LIBRARY_NAME Prism_Context)

set(CONTEXT_SOURCES
    context.cpp
)

set(CONTEXT_HEADERS
    public/context/context.hpp
)

add_library(${PRISM_CONTEXT_LIBRARY_NAME} STATIC
    ${CONTEXT_SOURCES}
    ${CONTEXT_HEADERS}
)

add_dependencies(
    ${PRISM_CONTEXT_LIBRARY_NAME}
        Prism_Loaders
        Prism_Controllers
        Prism_Shaders
        Prism_Systems
)

target_link_libraries(${PRISM_CONTEXT_LIBRARY_NAME}
    PRIVATE
        Prism_Loaders
        Prism_Controllers
        Prism_Systems
)

target_include_directories(${PRISM_CONTEXT_LIBRARY_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src 
)