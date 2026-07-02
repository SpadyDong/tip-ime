set(TIP_STATE_SOURCES
    input_state.cpp
)

add_library(tip_state STATIC ${TIP_STATE_SOURCES})

target_include_directories(tip_state
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
