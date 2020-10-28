cmake_minimum_required(VERSION 3.5)
project(plugin C CXX)

set(CMAKE_CXX_STANDARD 11)

if(MSVC)
    string(REGEX REPLACE "/W[1-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX /wd4480 /wd4725 /wd4127 /wd4100")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
endif()

include_directories(
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/.."
)

add_library(
    ${PROJECT_NAME} SHARED
    main.cpp
    patches.cpp
    plugin.rc
    plugin.def
    ../Shared/Common.cpp
    ../Shared/Hook.cpp
    ../Shared/hde/hde32.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".dll")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "plugin")
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
target_link_libraries(${PROJECT_NAME}
    kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32
    ole32 oleaut32 uuid odbc32 odbccp32 version
)
