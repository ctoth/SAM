cmake_minimum_required(VERSION 3.20.0)

set (PROJECT_VERSION "0.1")

project(Sam VERSION ${PROJECT_VERSION})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_executable(Sam src/cli.c src/phoneme_parser.c src/reciter.c src/sam.c src/render.c src/rules.c src/sam_tabs.c src/transitions.c src/wav.c)

add_library(samlib SHARED src/phoneme_parser.c src/render.c src/reciter.c src/sam_tabs.c  src/rules.c src/sam.c src/transitions.c)

set_target_properties(samlib
PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
)
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    # G++
    target_compile_options(samlib PRIVATE -Wall -Wextra)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    # MSVC
    target_compile_options(samlib PRIVATE /EHsc /MTd /W2 /c)
    # Set the DLLEXPORT variable to export symbols
    target_compile_definitions(samlib PRIVATE WIN_EXPORT)
endif()