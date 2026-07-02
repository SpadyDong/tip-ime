add_subdirectory(pinyin)
add_subdirectory(dictionary)
add_subdirectory(config)
add_subdirectory(state)

set(TIP_ENGINE_SOURCES
    input_processor.cpp
)

add_library(tip_engine STATIC ${TIP_ENGINE_SOURCES})

target_include_directories(tip_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tip_engine
    PUBLIC
        tip_pinyin
        tip_dictionary
        tip_config
        tip_state
        tip_utils
)
