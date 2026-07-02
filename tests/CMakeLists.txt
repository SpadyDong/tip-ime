add_subdirectory(unit)

# Integration tests require Windows runtime; skipped on non-Windows platforms.
if(WIN32)
    add_subdirectory(integration)
endif()
