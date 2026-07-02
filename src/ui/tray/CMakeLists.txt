set(TIP_TRAY_SOURCES
    tray_icon.cpp
)

add_library(tip_tray STATIC ${TIP_TRAY_SOURCES})

target_include_directories(tip_tray
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
