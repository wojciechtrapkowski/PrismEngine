cmake_minimum_required(VERSION 3.14)
project(Prism_Controllers VERSION 1.0.0 LANGUAGES CXX)

set(PRISM_CONTROLLERS_LIBRARY_NAME Prism_Controllers)

set(CONTROLLERS_SOURCES
)

set(CONTROLLERS_HEADERS
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
        Prism_Utils
)

target_include_directories(${PRISM_CONTROLLERS_LIBRARY_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)