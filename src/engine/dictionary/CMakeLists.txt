set(TIP_DICTIONARY_SOURCES
    trie_index.cpp
    dictionary.cpp
)

add_library(tip_dictionary STATIC ${TIP_DICTIONARY_SOURCES})

target_include_directories(tip_dictionary
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tip_dictionary
    PUBLIC
        tip_utils
)
