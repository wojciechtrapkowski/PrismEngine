set(STB_DIR ".")

set(STB_SOURCES)

set(STB_HEADERS
    "${STB_DIR}/stb_image.h"
)

add_library(stb INTERFACE ${STB_SOURCES})

target_include_directories(stb
    INTERFACE
        ${STB_DIR}
)
