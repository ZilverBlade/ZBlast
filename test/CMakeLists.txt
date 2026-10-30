add_executable(zblast_tests
    lexer_test.cpp
    parser_test.cpp
    ast_test.cpp
)

target_compile_definitions(zblast_tests PRIVATE
    TEST_FILES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../test_files"
)

target_include_directories(zblast_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    "${CMAKE_CURRENT_BINARY_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(zblast_tests PRIVATE
    zblast_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(zblast_tests)
