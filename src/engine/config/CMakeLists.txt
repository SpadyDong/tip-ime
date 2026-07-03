set(TIP_CONFIG_SOURCES
    config_manager.cpp
)

add_library(tip_config STATIC ${TIP_CONFIG_SOURCES})

target_include_directories(tip_config
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${TIP_SOURCE_DIR}
)

target_link_libraries(tip_config
    PUBLIC
        tip_utils
)
