find_program(ZBLAST_EXECUTABLE zblast
        PATHS "${CMAKE_BINARY_DIR}"
        NO_DEFAULT_PATH
        REQUIRED
)

set(ZBLAST_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/test.zast")
set(ZBLAST_AST_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test_ast")
set(ZBLAST_VISITOR_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test_visitor")

add_custom_command(
        OUTPUT ${ZBLAST_AST_OUTPUT}.cpp ${ZBLAST_AST_OUTPUT}.hpp
        DEPENDS ${ZBLAST_INPUT}
        COMMAND ${ZBLAST_EXECUTABLE} -a ${ZBLAST_INPUT} ${ZBLAST_AST_OUTPUT}
        COMMENT "Generating test_ast"
        VERBATIM
)
add_custom_command(
        OUTPUT ${ZBLAST_VISITOR_OUTPUT}.cpp ${ZBLAST_VISITOR_OUTPUT}.hpp
        DEPENDS ${ZBLAST_INPUT}
        COMMAND ${ZBLAST_EXECUTABLE} -v ${ZBLAST_INPUT} ${ZBLAST_VISITOR_OUTPUT}
        COMMENT "Generating test_visitor"
        VERBATIM
)


add_executable(zblast_example
    main.cpp
    ${ZBLAST_AST_OUTPUT}.cpp
    ${ZBLAST_VISITOR_OUTPUT}.cpp
)

target_include_directories(zblast_example PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
)
