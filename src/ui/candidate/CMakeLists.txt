set(TIP_CANDIDATE_SOURCES
    candidate_window.cpp
)

add_library(tip_candidate STATIC ${TIP_CANDIDATE_SOURCES})

target_include_directories(tip_candidate
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${TIP_SOURCE_DIR}
)

target_link_libraries(tip_candidate
    PUBLIC
        tip_config
        tip_utils
)

if(WIN32)
    target_link_libraries(tip_candidate PUBLIC gdi32 user32)
endif()
