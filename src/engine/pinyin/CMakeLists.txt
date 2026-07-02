set(TIP_PINYIN_SOURCES
    pinyin_parser.cpp
    fuzzy_sound.cpp
)

add_library(tip_pinyin STATIC ${TIP_PINYIN_SOURCES})

target_include_directories(tip_pinyin
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tip_pinyin
    PUBLIC
        tip_utils
)
