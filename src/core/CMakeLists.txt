set(TIP_CORE_SOURCES
    dllmain.cpp
    TextService.cpp
    KeyEventSink.cpp
    ComRegister.cpp
)

add_library(tip_ime SHARED ${TIP_CORE_SOURCES})

target_include_directories(tip_ime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${TIP_SOURCE_DIR}
)

target_link_libraries(tip_ime
    PRIVATE
        tip_engine
        tip_utils
        tip_candidate
)

if(MSVC)
    target_link_libraries(tip_ime PRIVATE ole32 uuid advapi32 shell32)
    # Newer Windows SDKs do not ship msctf.lib; the TSF GUIDs we need are
    # provided by uuid.lib. Only link msctf.lib if it is actually present.
    find_library(TIP_MSCTF_LIB msctf)
    if(TIP_MSCTF_LIB)
        target_link_libraries(tip_ime PRIVATE ${TIP_MSCTF_LIB})
    endif()
endif()

set_target_properties(tip_ime PROPERTIES
    RUNTIME_OUTPUT_NAME "tip_ime"
    LIBRARY_OUTPUT_NAME "tip_ime"
)
