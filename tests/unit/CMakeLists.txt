# Use a simple built-in test runner for the skeleton stage.
# Will be replaced by Google Test once third_party/gtest is integrated.

set(TIP_UNIT_TEST_SOURCES
    test_main.cpp
    test_pinyin_parser.cpp
    test_trie_index.cpp
    test_dictionary.cpp
    test_input_state.cpp
    test_input_processor.cpp
    test_string_utils.cpp
    test_config_manager.cpp
)

# Copy the project's data directory into the build root so that relative paths
# such as "data/dictionary/base_dict.txt" resolve when tests run from there.
file(COPY ${TIP_DATA_DIR} DESTINATION ${CMAKE_BINARY_DIR})

add_executable(tip_unit_tests ${TIP_UNIT_TEST_SOURCES})

target_include_directories(tip_unit_tests
    PRIVATE
        ${TIP_SOURCE_DIR}
)

target_link_libraries(tip_unit_tests
    PRIVATE
        tip_engine
        tip_utils
)

add_test(NAME tip_unit_tests COMMAND tip_unit_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
