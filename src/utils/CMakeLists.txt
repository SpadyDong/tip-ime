set(TIP_UTILS_SOURCES
    string_utils.cpp
    logger.cpp
)

add_library(tip_utils STATIC ${TIP_UTILS_SOURCES})

target_include_directories(tip_utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
