cmake_minimum_required(VERSION 3.20)
project(TIPInputMethod
    VERSION 0.1.0
    DESCRIPTION "TIP输入法 - 基于 TSF 的 Windows 拼音输入法"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TIP_BUILD_CORE "Build TSF core module (Windows only)" ON)
option(TIP_BUILD_UI "Build UI modules (Windows only)" ON)
option(TIP_BUILD_TESTS "Build unit tests and integration tests" ON)
option(TIP_ENABLE_WARNINGS "Enable strict compiler warnings" ON)

if(MSVC)
    add_compile_definitions(UNICODE _UNICODE NOMINMAX)
    add_compile_options(/utf-8)
endif()

if(TIP_ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options(/W4 /WX-)
    else()
        add_compile_options(-Wall -Wextra -Wpedantic)
    endif()
endif()

set(TIP_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TIP_SOURCE_DIR ${TIP_ROOT_DIR}/src)
set(TIP_DATA_DIR ${TIP_ROOT_DIR}/data)
set(TIP_THIRD_PARTY_DIR ${TIP_ROOT_DIR}/third_party)

add_subdirectory(src)

if(TIP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
