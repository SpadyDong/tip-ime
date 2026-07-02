add_subdirectory(utils)
add_subdirectory(engine)

if(WIN32 AND TIP_BUILD_CORE)
    add_subdirectory(core)
else()
    message(STATUS "TSF core module is only built on Windows.")
endif()

if(WIN32 AND TIP_BUILD_UI)
    add_subdirectory(ui)
else()
    message(STATUS "UI modules are only built on Windows.")
endif()
