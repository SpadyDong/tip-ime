set(TIP_CANDIDATE_SOURCES
    candidate_window.cpp
)

add_library(tip_candidate STATIC ${TIP_CANDIDATE_SOURCES})

target_include_directories(tip_candidate
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
