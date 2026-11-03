include_guard(GLOBAL)

# Internal: generate + attach sources for one .zast file.
function(_zblast_add target scope input)
    set(gen_dir "${CMAKE_CURRENT_BINARY_DIR}/zast/${target}")
    file(MAKE_DIRECTORY "${gen_dir}")

    cmake_path(ABSOLUTE_PATH input NORMALIZE) # relative to CMAKE_CURRENT_SOURCE_DIR
    cmake_path(GET input STEM name)

    set(ast "${gen_dir}/${name}_ast")
    set(outputs ${ast}.cpp ${ast}.hpp)

    add_custom_command(
            OUTPUT  ${ast}.cpp ${ast}.hpp
            COMMAND zblast::zblast -a "${input}" "${ast}"
            DEPENDS "$<TARGET_FILE:zblast::zblast>" "${input}"
            COMMENT "zblast: generating ${name}_ast"
            VERBATIM
    )

    if(ZBLAST_GENERATE_VISITORS)
        set(visitor "${gen_dir}/${name}_visitor")
        list(APPEND outputs ${visitor}.cpp ${visitor}.hpp)
        add_custom_command(
                OUTPUT  ${visitor}.cpp ${visitor}.hpp
                COMMAND zblast::zblast -v "${input}" "${visitor}"
                DEPENDS "$<TARGET_FILE:zblast::zblast>" "${input}"
                COMMENT "zblast: generating ${name}_visitor"
                VERBATIM
        )
    endif()

    target_sources(${target} ${scope} ${outputs})
    target_include_directories(${target} ${scope} "$<BUILD_INTERFACE:${gen_dir}>")

    # PUBLIC/INTERFACE: other directories will compile these files, but a
    # custom command's rule is only visible in the directory that created it.
    # A custom target carries the rule, and linking to `target` orders the build.
    if(NOT scope STREQUAL "PRIVATE")
        set(gen_target "zblast_${target}_${name}")
        add_custom_target(${gen_target} DEPENDS ${outputs})
        add_dependencies(${target} ${gen_target})
        set_source_files_properties(${outputs} PROPERTIES GENERATED TRUE)
    endif()
endfunction()

# target_zblast_sources(<target> <PRIVATE|PUBLIC|INTERFACE> files... [<scope> files...]...)
function(target_zblast_sources target)
    set(scope "")
    foreach(arg IN LISTS ARGN)
        if(arg MATCHES "^(PRIVATE|PUBLIC|INTERFACE)$")
            set(scope "${arg}")
            continue()
        endif()
        if(scope STREQUAL "")
            message(FATAL_ERROR
                "target_zblast_sources: expected PRIVATE, PUBLIC or INTERFACE before '${arg}'")
        endif()
        _zblast_add(${target} ${scope} "${arg}")
    endforeach()
endfunction()