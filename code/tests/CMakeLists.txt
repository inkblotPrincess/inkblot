include(GoogleTest)

add_executable(inkblot_tests

)

target_link_libraries(inkblot_tests
    PRIVATE
        inkblot::inkblot
        inkblot::options
        GTest::gtest_main
)

gtest_discover_tests(inkblot_tests)