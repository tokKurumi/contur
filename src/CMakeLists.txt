cmake_minimum_required(VERSION 3.20)
project(contur2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Tracing (compile-time toggle) ---
option(CONTUR2_ENABLE_TRACING "Enable kernel tracing (Debug only)" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CONTUR2_ENABLE_TRACING ON CACHE BOOL "" FORCE)
endif()

# --- contur2_lib static library ---
# Collects all .cpp files under contur/ (mirrors include/contur/ headers)
file(GLOB_RECURSE CONTUR2_LIB_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/contur/*.cpp
)

# If no .cpp files exist yet, create a header-only INTERFACE target temporarily,
# then switch to STATIC once sources appear.
if(CONTUR2_LIB_SOURCES)
    add_library(contur2_lib STATIC ${CONTUR2_LIB_SOURCES})
else()
    # No sources yet — use an INTERFACE library so linking still works
    add_library(contur2_lib INTERFACE)
endif()

# Public include path: src/include/
# Usage: #include "contur/core/types.h"
if(CONTUR2_LIB_SOURCES)
    target_include_directories(contur2_lib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    if(CONTUR2_ENABLE_TRACING)
        target_compile_definitions(contur2_lib PUBLIC CONTUR_TRACE_ENABLED)
    endif()
else()
    target_include_directories(contur2_lib INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    if(CONTUR2_ENABLE_TRACING)
        target_compile_definitions(contur2_lib INTERFACE CONTUR_TRACE_ENABLED)
    endif()
endif()

# --- Subdirectories ---
add_subdirectory(demos)
add_subdirectory(app)

# --- Tests (optional) ---
option(CONTUR2_BUILD_TESTS "Build unit and integration tests" ON)
if(CONTUR2_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# --- Doxygen Documentation ---
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_HTML_OUTPUT "html")
    doxygen_add_docs(docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/app
        COMMENT "Generating API documentation with Doxygen"
    )
    message(STATUS "Doxygen documentation target 'docs' available")
else()
    message(WARNING "Doxygen not found - documentation target will not be available")
endif()
