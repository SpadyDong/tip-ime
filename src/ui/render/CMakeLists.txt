set(TIP_RENDER_SOURCES
    d2d_renderer.cpp
)

add_library(tip_render STATIC ${TIP_RENDER_SOURCES})

target_include_directories(tip_render
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
