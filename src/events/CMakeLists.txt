cmake_minimum_required(VERSION 3.12)
project(Prism_Events VERSION 1.0.0 LANGUAGES CXX)

set(PRISM_EVENTS_LIBRARY_NAME Prism_Events)

set(EVENTS_HEADERS
    public/events/move_events.hpp
    public/events/window_resize_event.hpp
)

add_library(${PRISM_EVENTS_LIBRARY_NAME} INTERFACE
    ${EVENTS_HEADERS}
)

add_dependencies(
    ${PRISM_EVENTS_LIBRARY_NAME}
        EnTT
)

target_link_libraries(${PRISM_EVENTS_LIBRARY_NAME}
    INTERFACE
        EnTT
)

target_include_directories(${PRISM_EVENTS_LIBRARY_NAME}
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/public
)