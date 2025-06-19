# Define shader directories
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SHADER_BINARY_DIR "${CMAKE_BINARY_DIR}/bin/shaders")

# Create output directory
file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

# Find all shader files
file(GLOB SHADER_FILES "${SHADER_SOURCE_DIR}/*.vert" "${SHADER_SOURCE_DIR}/*.frag")

# Copy each shader file to the bin/shaders directory
foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(FILENAME ${SHADER_FILE} NAME)
    set(OUTPUT "${SHADER_BINARY_DIR}/${FILENAME}")
    
    add_custom_command(
        OUTPUT ${OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E copy ${SHADER_FILE} ${OUTPUT}
        DEPENDS ${SHADER_FILE}
        COMMENT "Copying shader ${FILENAME} to bin/shaders"
    )
    list(APPEND SHADER_COPIED_FILES ${OUTPUT})
endforeach()


# Custom target to copy shader files at build time
add_custom_target(Prism_Shaders ALL DEPENDS ${SHADER_COPIED_FILES})

target_sources(Prism_Shaders PRIVATE ${SHADER_FILES})