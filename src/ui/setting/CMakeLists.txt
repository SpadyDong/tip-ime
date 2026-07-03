set(TIP_SETTING_SOURCES
    setting_window.cpp
)

if(WIN32)
    add_executable(tip_setting WIN32 ${TIP_SETTING_SOURCES})
    target_include_directories(tip_setting
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${TIP_SOURCE_DIR}
    )
    target_link_libraries(tip_setting
        PRIVATE
            tip_config
            tip_candidate
            tip_utils
            comctl32
            gdi32
            user32
    )
endif()
