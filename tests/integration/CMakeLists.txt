# Integration tests for the full TSF input flow.
# These tests are only built on Windows.

set(TIP_INTEGRATION_SOURCES
    integration_stub.cpp
)

add_executable(tip_integration_tests ${TIP_INTEGRATION_SOURCES})

target_include_directories(tip_integration_tests
    PRIVATE
        ${TIP_SOURCE_DIR}
)
