cmake_minimum_required(VERSION 3.27)
project(ZBlast)

set(CMAKE_CXX_STANDARD 20)

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

add_executable(ZBlast
    src/main.cpp
    ${LEXER_OUTPUT}
)

target_include_directories(ZBlast PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")