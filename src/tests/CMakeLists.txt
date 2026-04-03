include(GoogleTest)
find_package(GTest REQUIRED)

set(CONTUR2_TEST_MAIN_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp)

# Unit tests
file(GLOB CONTUR2_UNIT_TESTS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp
)

if(CONTUR2_UNIT_TESTS)
    add_executable(contur2_unit_tests ${CONTUR2_TEST_MAIN_SOURCE} ${CONTUR2_UNIT_TESTS})
    target_link_libraries(contur2_unit_tests PRIVATE contur2_lib gtest::gtest)
    gtest_discover_tests(contur2_unit_tests)
endif()

# Integration tests
file(GLOB CONTUR2_INTEGRATION_TESTS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/integration/*.cpp
)

if(CONTUR2_INTEGRATION_TESTS)
    add_executable(contur2_integration_tests ${CONTUR2_TEST_MAIN_SOURCE} ${CONTUR2_INTEGRATION_TESTS})
    target_link_libraries(contur2_integration_tests PRIVATE contur2_lib gtest::gtest)
    gtest_discover_tests(contur2_integration_tests)
endif()
