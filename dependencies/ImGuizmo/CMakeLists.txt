set(IMGUIZMO_DIR ".")

set(IMGUIZMO_SOURCES
    "${IMGUIZMO_DIR}/ImGuizmo.cpp"
    "${IMGUIZMO_DIR}/GraphEditor.cpp"
    "${IMGUIZMO_DIR}/ImCurveEdit.cpp"
    "${IMGUIZMO_DIR}/ImGradient.cpp"
    "${IMGUIZMO_DIR}/ImSequencer.cpp"
)

add_library(imguizmo STATIC ${IMGUIZMO_SOURCES})

target_include_directories(imguizmo
    PUBLIC
        ${IMGUIZMO_DIR}
)

target_link_libraries(imguizmo
    PUBLIC imgui
)