# Create an executable from main.cpp
add_executable(PrismMain main.cpp)

# Add subdirectories
add_subdirectory(context)
add_subdirectory(loaders)
add_subdirectory(controllers)
add_subdirectory(shaders)
add_subdirectory(resources)
add_subdirectory(systems)
add_subdirectory(models)
add_subdirectory(events)
add_subdirectory(components)

# Link required libraries
target_link_libraries(PrismMain
    PRIVATE
       ${PRISM_DEPENDENCIES}
        Prism_Context
)


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