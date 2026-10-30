cmake_minimum_required(VERSION 3.27)
project(ZBlast)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(RE2C_EXECUTABLE re2c REQUIRED)

set(LEXER_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.re")
set(LEXER_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp")

add_custom_command(
    OUTPUT ${LEXER_OUTPUT}
    DEPENDS ${LEXER_INPUT}
    COMMAND ${RE2C_EXECUTABLE} -o ${LEXER_OUTPUT} ${LEXER_INPUT}
    COMMENT "Generating lexer.cpp"
    VERBATIM
)

add_library(zblast_core STATIC
    ${LEXER_OUTPUT}
    src/parser.cpp
)
target_include_directories(zblast_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(zblast
    src/main.cpp
)
target_link_libraries(zblast PRIVATE zblast_core)

if(MSVC)
    target_compile_options(zblast_core PRIVATE /WX)
    target_compile_options(zblast PRIVATE /WX)
else()
    target_compile_options(zblast_core PRIVATE -Werror)
    target_compile_options(zblast PRIVATE -Werror)
endif()

enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(test)

