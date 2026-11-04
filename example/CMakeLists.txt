set(ZBLAST_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/test.zast")
set(ZBLAST_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

set(ZBLAST_AST_OUTPUT "${ZBLAST_OUT_DIR}/test_ast")
set(ZBLAST_VISITOR_OUTPUT "${ZBLAST_OUT_DIR}/test_visitors")

add_custom_command(
    OUTPUT
        ${ZBLAST_AST_OUTPUT}.hpp ${ZBLAST_AST_OUTPUT}.cpp
        ${ZBLAST_VISITOR_OUTPUT}.hpp ${ZBLAST_VISITOR_OUTPUT}.cpp
    COMMAND zblast -g ast,visitor -d "${ZBLAST_OUT_DIR}" "${ZBLAST_INPUT}"
    MAIN_DEPENDENCY ${ZBLAST_INPUT}
    DEPENDS zblast
    COMMENT "Generating test AST and visitors"
    VERBATIM
)

add_executable(zblast_example
    main.cpp
    ${ZBLAST_AST_OUTPUT}.cpp
    ${ZBLAST_VISITOR_OUTPUT}.cpp
)

target_include_directories(zblast_example PRIVATE
        "${ZBLAST_OUT_DIR}"
)