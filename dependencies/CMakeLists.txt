# Dependencies CMakeLists.txt
add_subdirectory(entt)
add_subdirectory(glfw)
add_subdirectory(imgui)
add_subdirectory(glm)
add_subdirectory(assimp)

# Create GLAD library
add_library(glad STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c")

target_include_directories(glad
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Find OpenGL
find_package(OpenGL REQUIRED)

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

message(STATUS "GLAD library configured successfully")


set(PRISM_DEPENDENCIES 
    glad
    glfw
    glm
    imgui
    ${OPENGL_LIBRARIES}
    ${MAC_LIBRARIES}
    PARENT_SCOPE
)