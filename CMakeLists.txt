cmake_minimum_required(VERSION 3.21)

project(ets2util LANGUAGES C)

add_library(${PROJECT_NAME} SHARED 
    "src/main.h"
    "src/main.c"
    "src/init.h"
    "src/init.c"
    "src/util/logging.h"
    "src/util/logging.c"
    "src/util/pattern/pattern.h"
    "src/util/pattern/pattern.c"
    "src/util/pattern/pattern_scan.c"
    "src/util/pattern/pattern_scan.h" "src/util/pointer.h")

target_include_directories(${PROJECT_NAME} PUBLIC "src/")

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED YES
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /OX
        /Oi
        /GF
        /Gy
        /TC
        /std:c17
        /Wall
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        /SUBSYSTEM:WINDOWS
        /INCREMENTAL:NO
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _MBCS
        _WIN32_WINNT=0x601
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
    )
endif()