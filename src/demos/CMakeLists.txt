# Demos library — linked into the main app
file(GLOB_RECURSE CONTUR2_DEMO_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

if(CONTUR2_DEMO_SOURCES)
    add_library(contur2_demos STATIC ${CONTUR2_DEMO_SOURCES})
else()
    add_library(contur2_demos INTERFACE)
endif()

if(CONTUR2_DEMO_SOURCES)
    target_include_directories(contur2_demos PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    target_link_libraries(contur2_demos PUBLIC contur2_lib)

    # Step-by-step mode in Debug builds
    target_compile_definitions(contur2_demos
        PRIVATE
            $<$<CONFIG:Debug>:CONTUR_STEP_MODE>
    )
else()
    target_include_directories(contur2_demos INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
endif()
