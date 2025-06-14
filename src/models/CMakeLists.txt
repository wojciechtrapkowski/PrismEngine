# Define MODELS directories
set(MODELS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODELS_BINARY_DIR "${CMAKE_BINARY_DIR}/bin/models")

# Create output directory
file(MAKE_DIRECTORY ${MODELS_BINARY_DIR})

# Find all Assimp-supported model files
file(GLOB MODELS_FILES
    "${MODELS_SOURCE_DIR}/*.obj"
    "${MODELS_SOURCE_DIR}/*.fbx"
    "${MODELS_SOURCE_DIR}/*.dae"
    "${MODELS_SOURCE_DIR}/*.3ds"
    "${MODELS_SOURCE_DIR}/*.blend"
    "${MODELS_SOURCE_DIR}/*.ply"
    "${MODELS_SOURCE_DIR}/*.stl"
    "${MODELS_SOURCE_DIR}/*.gltf"
    "${MODELS_SOURCE_DIR}/*.glb"
    "${MODELS_SOURCE_DIR}/*.x"
    "${MODELS_SOURCE_DIR}/*.ac"
    "${MODELS_SOURCE_DIR}/*.ms3d"
    "${MODELS_SOURCE_DIR}/*.lwo"
    "${MODELS_SOURCE_DIR}/*.cob"
    "${MODELS_SOURCE_DIR}/*.ifc"
    "${MODELS_SOURCE_DIR}/*.off"
    "${MODELS_SOURCE_DIR}/*.3d"
    "${MODELS_SOURCE_DIR}/*.ase"
)

# Copy each model file to the bin/models directory
foreach(MODELS_FILE ${MODELS_FILES})
    get_filename_component(FILENAME ${MODELS_FILE} NAME)
    set(OUTPUT "${MODELS_BINARY_DIR}/${FILENAME}")
    
    add_custom_command(
        OUTPUT ${OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E copy ${MODELS_FILE} ${OUTPUT}
        DEPENDS ${MODELS_FILE}
        COMMENT "Copying model file ${FILENAME} to bin/models"
    )
    list(APPEND MODELS_COPIED_FILES ${OUTPUT})
endforeach()

# Create a custom target that depends on all copied model files
add_custom_target(Prism_Models ALL DEPENDS ${MODELS_COPIED_FILES})