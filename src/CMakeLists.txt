# Create an executable from main.cpp
add_executable(PrismMain main.cpp)

# Link required libraries
target_link_libraries(PrismMain
    PRIVATE
        ${OPENGL_LIBRARIES}
        glad
        imgui
        EnTT::EnTT
)

# If using GLFW as a target
if(TARGET glfw)
    target_link_libraries(PrismMain PRIVATE glfw)
else()
    # If GLFW was found as a package or is in dependencies
    find_library(GLFW_LIBRARY glfw NAMES glfw3 libglfw libglfw3 PATHS ${DEPENDENCIES_LIBRARY_DIR})
    if(GLFW_LIBRARY)
        target_link_libraries(PrismMain PRIVATE ${GLFW_LIBRARY})
    else()
        message(WARNING "GLFW library not found")
    endif()
endif()

# Mac-specific configurations
if(APPLE)
    target_link_libraries(PrismMain PRIVATE ${MAC_LIBRARIES})
endif()

# Set the output directory
set_target_properties(PrismMain
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install the executable
install(TARGETS PrismMain
    RUNTIME DESTINATION bin
)

message(STATUS "Temporary build setup configured for main.cpp")