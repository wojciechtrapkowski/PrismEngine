# Dependencies CMakeLists.txt

add_subdirectory(entt)
add_subdirectory(glfw)
add_subdirectory(imgui)

# Create a target for GLAD

# Create GLAD library
add_library(glad STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c")

# Set include directories for GLAD
target_include_directories(glad
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link with OpenGL
target_link_libraries(glad
    PUBLIC
        ${OPENGL_LIBRARIES}
)
    
message(STATUS "GLAD library configured successfully")

# Mac-specific configurations
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    
    set(MAC_LIBRARIES
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
    )
endif()